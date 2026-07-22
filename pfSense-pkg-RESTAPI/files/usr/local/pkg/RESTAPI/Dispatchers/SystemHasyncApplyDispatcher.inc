<?php

namespace RESTAPI\Dispatchers;

use RESTAPI\Core\Dispatcher;

/**
 * Defines a Dispatcher for applying changes to the HA Sync configuration.
 */
class SystemHasyncApplyDispatcher extends Dispatcher {
    /**
     * Reloads the HA Sync and filter services.
     */
    protected function _process(mixed ...$arguments): void {
        if ($this->async) {
            filter_configure();
        } else {
            filter_configure_sync();
        }
    }
}
