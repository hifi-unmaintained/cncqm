var cncqm_session = false;
var cncqm_host = false;

function cncqm_options_load(data)
{
    $('#cncqm div.options div.options_common').show();

    /* update current player info */
    for (var i in data) {
        if ( (i == 'host' || i == 'client') && data[i] != null ) {
            var plr = data[i];
            if (plr.id == cncqm_session) {
                $('#cncqm div.options div.options_common .handle').html(plr.handle);
                $('#cncqm div.options div.options_common .side').html(plr.side);
                $('#cncqm div.options div.options_common .color').html(plr.color);
            }
        }
    }

    if (cncqm_host) {
        for (var i in data) {
            $('#cncqm div.options div.options_host .'+i).val(data[i]);
        }
        $('#cncqm div.options div.options_host').show();
    } else {
        for (var i in data) {
            $('#cncqm div.options div.options_host .'+i).html(data[i]);
        }
        $('#cncqm div.options div.options_client').show();
    }
}

function cncqm_ajax_cb(data, status, xhr)
{
    /* stop all loading icons */
    $('#cncqm div.options').css('background-position', '-100px -100px');

    for (var i in data) {
        var row = data[i];

        if (row.reply == 'error') {
            alert('Unexpected AJAX error: '+row.message);
            return;
        }

        if (row.reply == 'session') {
            cncqm_session = row.id;
            cncqm_host = row.host;
        }

        if (row.reply == 'info') {
            cncqm_options_load(row.game);
        }
    }
}

function cncqm_ajax_error(data, status, xhr)
{
    alert('Unexpected connection error, FFFFUUUUUUU!!');
}

$().ready(function() {
    $('#cncqm_button_host').click(function() {
        $('#cncqm div.options div.host').hide();
        $('#cncqm div.options').css('background-position', '50% 50%');

        $.ajax({
            data : {
                action : 'host',
                handle : $('#cncqm_input_handle').val()
            }
        });
    });   

    $('#cncqm div.options_host input, #cncqm div.options_host select').change(function() {
        $('#cncqm div.options div.options_host').hide();
        $('#cncqm div.options').css('background-position', '50% 50%');

        $.ajax({
            data : {
                action : 'change',
                session : cncqm_session,
                field : $(this).attr('name'),
                value : $(this).val()

            }
        });
    });

    $.ajaxSetup({
        url: 'api.php',
        dataType: 'json',
        success: cncqm_ajax_cb,
        error: cncqm_ajax_error
    });
});
