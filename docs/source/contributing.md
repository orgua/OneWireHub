# Contributing

Help is always welcome! Either by finding and [reporting Bugs](./help_me_help_you.md), suggesting new features or better by proposing already working code as pull request.

## Setting up the Dev-Environment

First clone and enter the project directory

```Shell
git clone https://github.com/orgua/OneWireHub.git
cd  OneWireHub
```

The project uses `pipenv` (and [Python](https://www.python.org/)) to provide a reproducible environment (optional).
First make sure `pipenv` is installed and after that initialize & enter the environment.
The most common shell-commands for Ubuntu are

```Shell
pip install pipenv
sudo apt install cppcheck
pipenv install
# later only the following cmd is needed:
pipenv shell
# to exit this environment send
exit
```

Your current command line should have an additional `(OneWireHub)` in front when inside the environment.

## Code-Quality

A set of linters and formatters are ensuring the quality of the codebase. Some of these change the code automatically, others only produce warnings. To run the tests execute

```Shell
pre-commit run -a
```

in the project directory.

## Unittests

There are several options you can test new code - see [Test](./test_setup.md)-Section for more details. In short

- try to test new code with a real physical OneWire-Host
- new peripheral device? please provide an [example](https://github.com/orgua/OneWireHub/tree/main/examples)
- a new example? please add it to the [compile-action](https://github.com/orgua/OneWireHub/blob/main/.github/workflows/compile.yml)
- compile-actions test code automatically when opening a PR

## Preparing a Pull Request

Before submitting code, please work through the previous two sections QA and Unittests. Your PR will probably be stalled when the tests fail as these will be run automatically by GitHub Actions.

## Preparing a new Version

Before releasing a new version, please work through the two sections QA and Unittests.

Increment Version and decide if changes justify major, minor or patch-release

```Shell
bump2version --tag patch
```

Assemble a changelog and add it to the [documentation](./changelog.md).
Issue a release on GitHub and attach the new changelog-section.
