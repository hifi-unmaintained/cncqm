<?php

class CnCQM_Format
{
    private $__obj;
    private $__type;
    private $__ret;

    function __construct($obj, $type = 'html')
    {
        $this->__obj = $obj;
        $this->__type = $type;
    }

    function &__get($k)
    {
        if ($this->__type == 'html') {
            $this->__ret = htmlspecialchars($this->__obj->$k);
        } else {
            $this->__ret = $this->__obj->$k;
        }

        return $this->__ret;
    }
}
