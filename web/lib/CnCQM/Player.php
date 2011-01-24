<?php

class CnCQM_Player extends CnCQM
{
    static $schema = array(
        'handle'        => array('type'     => 'string',
                                 'size'     => 8,
                                 'null'     => false),
        'color'         => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 0,
                                 'null'     => false),
        'side'          => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 0,
                                 'null'     => false),
        'address'       => array('type'     => 'string',
                                 'size'     => 16,
                                 'null'     => false),
        'port'          => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 5009,
                                 'null'     => false),
        'ready'         => array('type'     => 'boolean',
                                 'null'     => false,
                                 'default'  => false),
    );
}
