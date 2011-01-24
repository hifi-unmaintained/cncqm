<?php

class CnCQM_Game extends CnCQM
{
    static $schema = array(
        'status'        => array('type'     => 'string',
                                 'values'   => array('created', 'waiting', 'started'),
                                 'default'  => 'created'),
        'units'         => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 6,
                                 'null'     => false),
        'tech'          => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 3,
                                 'null'     => false),
        'credits'       => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 3000,
                                 'null'     => false),
        'bases'         => array('type'     => 'boolean',
                                 'default'  => true,
                                 'null'     => false),
        'crates'        => array('type'     => 'boolean',
                                 'default'  => false,
                                 'null'     => false),
        'tiberium'      => array('type'     => 'boolean',
                                 'default'  => false,
                                 'null'     => false),
        'ai'            => array('type'     => 'boolean',
                                 'default'  => false,
                                 'null'     => false),
        'scenario'      => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 1,
                                 'null'     => false),
        'maxahead'      => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 15,
                                 'null'     => false),
        'sendrate'      => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 5,
                                 'null'     => false),
        'seed'          => array('type'     => 'integer',
                                 'signed'   => false,
                                 'default'  => 0,
                                 'null'     => false),
        'host'          => array('type'     => 'has_many',
                                 'class'    => 'CnCQM_Player',
                                 'default'  => NULL,
                                 'null'     => true),
        'client'        => array('type'     => 'has_many',
                                 'class'    => 'CnCQM_Player',
                                 'default'  => NULL,
                                 'null'     => true),
        'created'       => array('type'     => 'datetime',
                                 'default'  => '0000-00-00 00:00:00',
                                 'readonly' => true,
                                 'null'     => true),
        'changed'       => array('type'     => 'datetime',
                                 'default'  => '0000-00-00 00:00:00',
                                 'readonly' => true,
                                 'null'     => true),
        'started'       => array('type'     => 'datetime',
                                 'default'  => NULL,
                                 'null'     => true),
    );

    function save()
    {
        if ($this->id === NULL) {
            $this->created = date('Y-m-d H:i:s');
        }

        $this->changed = date('Y-m-d H:i:s');

        parent::save();
    }
}
