# pfSense REST API Package

[![Build](https://github.com/pfrest/pfSense-pkg-RESTAPI/actions/workflows/build.yml/badge.svg)](https://github.com/pfrest/pfSense-pkg-RESTAPI/actions/workflows/build.yml)
[![Quality](https://github.com/pfrest/pfSense-pkg-RESTAPI/actions/workflows/quality.yml/badge.svg)](https://github.com/pfrest/pfSense-pkg-RESTAPI/actions/workflows/quality.yml)
[![Release](https://github.com/pfrest/pfSense-pkg-RESTAPI/actions/workflows/release.yml/badge.svg)](https://github.com/pfrest/pfSense-pkg-RESTAPI/actions/workflows/release.yml)<br>
![Downloads](https://img.shields.io/github/downloads/pfrest/pfSense-pkg-RESTAPI/total?label=Downloads)
![License](https://img.shields.io/github/license/pfrest/pfSense-pkg-RESTAPI?label=License)
![Docs](https://img.shields.io/website?url=https%3A%2F%2Fpfrest.org&label=Documentation)

The pfSense REST API package is an unofficial, open-source REST and GraphQL API for pfSense CE and pfSense Plus
firewalls. It is designed to be light-weight, fast, and easy to use. This guide will help you get started with the REST
API package and provide you with the information you need to configure and use the package effectively.

## Key Features

- 200+ REST endpoints available for managing your firewall and associated services
- A GraphQL API for flexible data retrieval and mutation
- Easy to use querying and filtering
- Configurable security settings
- Supports HATEOAS driven development
- Customizable authentication options
- Built-in Swagger documentation

## Getting Started

- [Installation and Configuration](https://pfrest.org/INSTALL_AND_CONFIG/)
- [Authentication and Authorization](https://pfrest.org/AUTHENTICATION_AND_AUTHORIZATION/)
- [Swagger and OpenAPI](https://pfrest.org/SWAGGER_AND_OPENAPI/)
- [Queries, Filters, and Sorting](https://pfrest.org/QUERIES_FILTERS_AND_SORTING/)

## Quickstart

For new users, it is recommended to refer to the links in the [Getting Started section](#getting-started) to begin. Otherwise, the installation
commands are included below for quick reference.

Install on pfSense CE:

```bash
pkg-static add https://github.com/pfrest/pfSense-pkg-RESTAPI/releases/latest/download/pfSense-2.8.1-pkg-RESTAPI.pkg
```

Install on pfSense Plus:

```bash
pkg-static -C /dev/null add https://github.com/pfrest/pfSense-pkg-RESTAPI/releases/latest/download/pfSense-26.03-pkg-RESTAPI.pkg
```

> [!WARNING]
> Before installing the package, always ensure your pfSense version is supported! Supported versions are listed [here](https://pfrest.org/INSTALL_AND_CONFIG/#supported-pfsense-versions).
> Installation of the package on unsupported versions of pfSense may result in unexpected behavior and/or system instability.

> [!TIP]
> You may need to customize the installation command to reference the package built for your pfSense version. Check
> the [releases page](https://github.com/pfrest/pfSense-pkg-RESTAPI/releases) to find the package built for
> your version of pfSense.

## Disclaimers

> [!CAUTION]
> This package is not affiliated or supported by Netgate or the pfSense team. This package is developed and maintained
> by the community.
