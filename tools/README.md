# Emoji Model builder

This folder contains the tool to build all emoji mapping.

## Requirements

* composer
* PHP

## Update the rules

To update the rules, you need to update the version of `unicode-org/cldr` in the
`composer.json` file, then run `make update`.

Finally, run the following command:

```bash
make build
```