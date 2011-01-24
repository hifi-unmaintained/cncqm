<?php
    require_once 'include/top.inc.php';
?>
        <h1>CnCQM - Command &amp; Conquer Quick Match</h1>
        <p>keepin' C&amp;C online</p>
        <div id="cncqm">
            <div class="players">
                <h2><span>Players</span></h2>
            </div>
            <div class="games">
                <h2><span>Games</span></h2>
            </div>
            <div class="chat">
                <h2><span>Quick Chat</span></h2>
                <pre>(TODO)</pre>
                <input type="text" />
            </div>
            <div class="options">
                <h2><span>Options</span></h2>
                <div class="host">
                    Name: <input id="cncqm_input_handle" class="text" value="" />
                    <button id="cncqm_button_host">Host game</button>
                </div>
                <div class="options_common" style="display: none">
                    Name: <span class="handle"></span>
                    Side:
                    <select name="side">
                        <option value="0">GDI</option>
                        <option value="1">NOD</option>
                    </select>
                    Color:
                    <select name="color">
                        <option value="0">Yellow</option>
                        <option value="1">Red</option>
                        <option value="2">Cyan</option>
                        <option value="3">Orange</option>
                        <option value="4">Green</option>
                        <option value="5">Gray</option>
                    </select>
                </div>
                <div class="options_host" style="display: none">
                    game options
                </div>
                <div class="options_client" style="display: none">
                    TODO (client)
                </div>
            </div>
        </div>
<?php
    require_once 'include/bottom.inc.php';
?>
