# HA Sync

The REST API package supports High Availability (HA) synchronization, allowing you to automatically replicate your
REST API settings from one pfSense host to one or more remote peers. This is especially useful in HA deployments where
you want all nodes to share the same REST API configuration without having to configure each host manually.

!!! Important
    HA sync only synchronizes the **REST API package settings** (such as authentication methods, allowed interfaces,
    JWT expiration, and other options found under the REST API settings). It does not synchronize the rest of the
    pfSense configuration. Use pfSense's built-in HA/XMLRPC configuration sync for the base system configuration.

## How It Works

When HA sync is enabled and one or more sync hosts are defined, the source host will push its REST API settings to each
configured peer whenever those settings change. Each peer receives the sync request and applies the incoming settings
to its own REST API configuration.

## Requirements

Before enabling HA sync, ensure the following:

- The REST API package is installed on every host that will participate in synchronization.
- A dedicated user exists on **every** peer defined in the sync hosts list. This user must hold the `page-all`
  privilege and must use the same username and password across all hosts.
- Each peer is reachable from the source host over HTTPS (the webConfigurator port).
- If certificate validation is enabled, each peer's webConfigurator must present a certificate trusted by the source
  host.

!!! Warning
    It is expected that the source host and each of its peers are as identical as possible. Changes between REST API 
    versions or pfSense versions may make the synced configurations incompatible with the remote peers. Additionally, 
    some settings (such as the allowed interfaces) are configuration-specific, and it is expected that those interfaces 
    also exist on each remote peer.

## Configuring HA Sync on the Source Host

Follow these steps on the host that will send its REST API settings to peers:

1. In the webConfigurator, navigate to **System > REST API > Settings**.
2. Enable the **HA Sync** toggle. This allows the host to both send sync requests to peers and accept incoming sync
   requests from peers.
3. In the **HA Sync Hosts** field, enter the IP address or hostname of each remote peer that should receive this host's
   REST API settings. You may define up to 10 hosts.
4. Set the **HA Sync Username** to the username of the account used to authenticate sync requests. This user must exist on
   every peer and hold the `page-all` privilege.
5. Set the **HA Sync Password** to the password for the HA username. This must match on all peers.
6. (Recommended) Leave **HA Sync Validate Certs** enabled to validate each peer's webConfigurator certificate. This
   helps mitigate Man-in-the-Middle attacks. If you disable it, certificate validation will be skipped.
7. Save your changes.

## Configuring HA Sync on Remote Peers

Repeat the following on **every** remote peer listed in the source host's **HA Sync Hosts** field:

1. In the webConfigurator, navigate to **System > REST API > Settings**.
2. Enable the **HA Sync** toggle. A peer will reject incoming sync requests by default unless the HA Sync toggle is enabled on that
   peer.
3. Ensure the HA sync user (with the `page-all` privilege) exists on the peer and uses the same username and password
   as configured on the source host.
4. Save your changes.

Once HA Sync is enabled on both the source host and all peers, any change to the source host's REST API settings will
automatically be synchronized to each configured peer.

## Configuring HA Sync via the REST API

All HA sync options are also available through the REST API settings endpoint, so you can automate HA sync configuration
as part of your provisioning workflow. The relevant settings include:

- `ha_sync` — Enables or disables HA sync on the host. Must be enabled on both the sending and receiving hosts.
- `ha_sync_validate_certs` — Enables or disables certificate validation when syncing to peers.
- `ha_sync_hosts` — A list of IP addresses or hostnames to sync REST API settings to.
- `ha_sync_username` — The username used to authenticate sync requests. Must hold the `page-all` privilege on all peers.
- `ha_sync_password` — The password for the HA sync user. Must match across all peers.
