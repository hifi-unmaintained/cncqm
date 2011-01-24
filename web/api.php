<?php

require_once 'include/init.inc.php';

$data = (object)$_GET;

if (!isset($data->action))
    exit;

header('Content-type: text/plain; charset=UTF-8');

$ret = array();

switch($data->action)
{
    case 'list':
        $ret = array('reply' => 'list', 'games' => array());
        foreach (CnCQM_Game::find('all') as $game) {
            $ret['games'][] = $game->toArray();
        }
        break;

    case 'info':
        $game = CnCQM_Game::find('one', array('id' => $data->id));

        $game_arr = $game->toArray();

        $game_arr['host'] = $game->host->toArray();

        if ($game->client) {
            $game_arr['client'] = $game->client->toArray();
        } else {
            $game_arr['client'] = NULL;
        }

        $ret[] = array(
            'reply' => 'info',
            'game' => $game_arr
        );
        break;

    case 'host':
        $game = new CnCQM_Game();
        $game->host = new CnCQM_Player(array('handle' => $data->handle, 'address' => $_SERVER['REMOTE_ADDR']));
        $game->host->save();
        $game->save();

        $game_arr = $game->toArray();
        $game_arr['host'] = $game->host->toArray();
        $game_arr['client'] = NULL;

        $ret[] = array(
            'reply' => 'session',
            'id' => $game->host->id,
            'host' => 1
        );

        $ret[] = array(
            'reply' => 'info',
            'game' => $game_arr
        );

        break;

    case 'change':
        $game = CnCQM_Game::find('one', array('where' => array('host_id = ? OR client_id = ?', $data->session, $data->session)));

        if ($game->host->id == $data->session) {
            $player = $game->host;
        } else {
            $player = $game->client;
        }

        if ($data->field == 'side' || $data->field == 'color') {
            $player->side = $data->value;
            $player->save();
        } else {
            $tmp = $data->field;
            $game->$tmp = $data->value;
        }

        $game->save();

        $game_arr = $game->toArray();
        $game_arr['host'] = $game->host->toArray();

        if ($game->client) {
            $game_arr['client'] = $game->client->toArray();
        } else {
            $game_arr['client'] = NULL;
        }

        $ret[] = array(
            'reply' => 'info',
            'game' => $game_arr
        );

        break;

    case 'join':
        break;
}

if (count($ret) == 0) {
    $ret[] = array('reply' => 'error', 'message' => 'No action handler');
}

echo json_encode($ret);
