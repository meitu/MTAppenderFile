#!/usr/bin/env bash

set -e

BASE_DIR=$(dirname "$0")

sh $BASE_DIR/Scripts/Hooks/setup-hook.sh

git config commit.template $BASE_DIR/.gitlab/git_commit_templates/Commit_Template.md
