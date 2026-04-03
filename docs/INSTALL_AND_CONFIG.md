# Installation and Configuration

## Requirements

### Hardware requirements

Overall, the REST API package is designed to be as lightweight as possible and should run fine on any hardware that can
run pfSense. It's recommended to follow Netgate's [minimum hardware requirements](https://docs.netgate.com/pfsense/en/latest/hardware/minimum-requirements.html).

!!! Warning
    - The package is currently not supported on 32-bit architectures like the Netgate 3100 (SG-3100).
    - While the package should behave identically on 64-bit architectures other than amd64, automated testing only covers amd64
    builds of pfSense CE. Support on other architectures is not guaranteed.

### Supported pfSense versions

- pfSense CE 2.8.0
- pfSense CE 2.8.1
- pfSense Plus 24.11
- pfSense Plus 25.07
- pfSense Plus 25.07.1
- pfSense Plus 25.11
- pfSense Plus 25.11.1
- pfSense Plus 26.03

!!! Warning
    Installation of the package on unsupported versions of pfSense may result in unexpected behavior and/or system instability.
    
!!! Tip
    Don't see your version of pfSense listed? Older versions of pfSense may be supported by older versions of this package.
    Check the [releases page](https://github.com/pfrest/pfSense-pkg-RESTAPI/releases).

## Installing the package

The pfSense REST API package is built just like any other pfSense package and can therefor be installed easily using
`pkg` from the pfSense command line. Below are the installation commands for the latest version of the package.

**Install on pfSense CE**

```bash
pkg-static add https://github.com/pfrest/pfSense-pkg-RESTAPI/releases/latest/download/pfSense-2.8.1-pkg-RESTAPI.pkg
```

**Install on pfSense Plus**

```bash
pkg-static -C /dev/null add https://github.com/pfrest/pfSense-pkg-RESTAPI/releases/latest/download/pfSense-25.11-pkg-RESTAPI.pkg
```

!!! Important
    - You may need to customize the installation command to reference the package built for your pfSense version. Check
      the [releases page](https://github.com/pfrest/pfSense-pkg-RESTAPI/releases) to find the package built for
      your version of pfSense.
    - When updating pfSense, **you must reinstall this package afterward** as pfSense removes unofficial packages during
      system updates and has no way to automatically reinstall them. 
    - If you're looking for a method of programmatically installing the package without SSH, check out 
      [pfsense-vshell](https://github.com/jaredhendrickson13/pfsense-vshell)!

## Configuring the package

The REST API is designed to be ready to use out of the box. However, there are a number of configuration options available to
you to customize the API to your needs. The REST API settings can be configured in pfSense webConfigurator under
'System' -> 'REST API' or via `PATCH` request to the [/api/v2/system/restapi/settings](https://pfrest.org/api-docs/#/SYSTEM/patchSystemRESTAPISettingsEndpoint)
endpoint.

## Deleting the package

To remove the REST API package from your pfSense instance, you can use the following command:

```bash
pfsense-restapi delete
```

!!! Note
    In the event that you are unable to use the `pfsense-restapi` command, you can manually remove the package by
    running `pkg-static delete pfSense-pkg-RESTAPI`.

## Updating the package

Before updating the package, it is recommended to enable the REST API's 'Keep Backup' setting to ensure that your
REST API configurations, keys and access lists are not lost during the update process. It is also highly recommended to
read and understand both the release change notes on GitHub and the [versioning policy](#versioning-policy) to ensure
you do not unintentionally introduce breaking changes to your environment.

!!! Tip
    While the package is updating, the REST API may be unavailable for a short period of time. Updates typically complete
    within a minute, but may vary depending on network environment and conditions. It is recommended to
    schedule updates during off-peak hours to minimize impact to your integrations. In the event that the update fails,
    or takes an excessive amount of time, it is recommended to uninstall and reinstall the package.

### From the pfSense webConfigurator

You can easily update or revert the package version from the pfSense webConfigurator by navigating to 'System' -> 
'REST API' -> 'Updates' and select the desired version. Click 'Save' to apply the desired version. 

### From the API

You can update or revert the package to a specified version by sending a request to the [PATCH 
/api/v2/system/restapi/version](https://pfrest.org/api-docs/#/SYSTEM/patchSystemRESTAPIVersionEndpoint) endpoint.
Simply set the `install_version` field to the desired version and send the request.

### From the command line

You can update the package to latest version available to your pfSense version by running the following command:

```bash
pfsense-restapi update
```

If you need to revert or upgrade the package to a _specific_ version, you can do so by running the following command:

```bash
pfsense-restapi revert <version>
```

## Versioning policy

The REST API package loosely follows [Semantic Versioning](https://semver.org/). The versioning policy is as follows:

- Major version changes (e.g. 1.x.x to 2.x.x) may include major breaking changes and are not guaranteed to be backwards
  compatible. Major changes will often include significant changes to the framework, endpoints, schemas, and/or behavior.
- Minor version changes (e.g. 2.0.x to 2.1.x) may include new features, bug fixes, and/or minor breaking changes. Breaking
  changes will be isolated to specific endpoints and will be documented in the release notes.
- Patch version changes (e.g. 2.0.0 to 2.0.1) will only include bug fixes and very small enhancements. Patches will
  never contain breaking changes or significant new features that could impact existing functionality.

### Pre-release versions

Pre-release versions are occasionally made available to the public to allow for testing of new features and fixes. 
Pre-releases will be notated as such on GitHub and are not considered as release candidates within the REST API package's
update features by default. You may opt-in to pre-release updates by enabling the 'Allow Pre-releases' setting in the
REST API settings.

!!! Warning
    Pre-release versions may contain breaking changes, bugs, and/or incomplete features. It is recommended to only use
    pre-release versions in testing environments and never in production.