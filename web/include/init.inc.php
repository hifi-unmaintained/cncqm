<?php

preg_match('!^(.+)/include!', __FILE__, $m);
define('ROOT_DIR', $m[1].'/');
unset($m);

set_include_path(get_include_path() . PATH_SEPARATOR . ROOT_DIR . 'include/' . PATH_SEPARATOR . ROOT_DIR . 'lib/');

function __autoload($class)
{
    $file = str_replace('_', '/', $class).'.php';

    if (file_exists(ROOT_DIR.'include/'.$file)) {
        include ROOT_DIR.'include/'.$file;
        return true;
    }

    if (file_exists(ROOT_DIR.'lib/'.$file)) {
        include ROOT_DIR.'lib/'.$file;
        return true;
    }

    return false;
}

define('DB', ROOT_DIR.'db/cncqm.db');

if (!file_exists(DB)) {
    YARR::init(new PDO('sqlite:'.DB));
    YARR::$db->exec(CnCQm_Player::schemaToSQL());
    YARR::$db->exec(CnCQm_Game::schemaToSQL());
} else {
    YARR::init(new PDO('sqlite:'.DB));
}
