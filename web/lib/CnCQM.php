<?php

class CnCQM extends YARR
{
    protected $__html;

    function __construct($array = array())
    {
        return parent::__construct($array);
    }

    function &__get($k)
    {
        if ($k == 'html') {
            if ($this->__html == NULL) {
                $this->__html = new CnCQM_Format($this, 'html');
            }
            return $this->__html;
        }

        return parent::__get($k);
    }
}
