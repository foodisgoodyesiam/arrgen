#!/usr/bin/env bash
# Copyright © 2024 Steven Marion <steven@dragons.fish>
#
# This file is part of arrgen.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the gnu general public license as published by
# the free software foundation, either version 3 of the license, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but without any warranty; without even the implied warranty of
# merchantability or fitness for a particular purpose.  see the
# gnu general public license for more details.
#
# you should have received a copy of the gnu general public license
# along with this file.  if not, see <https://www.gnu.org/licenses/>.
temp=$(mktemp)

if [[ -d '.git' ]]; then
	version_message="$(git log --pretty=format:'Commit: %H\n%cd\n' -n 1)"
	branch=$(git rev-parse --abbrev-ref HEAD)
	if ! [[ $branch == master ]]; then
		version_message="${version_message}Branch: $branch\n"
	fi
	git update-index --refresh
	if ! git diff-index --quiet HEAD --; then
		version_message="${version_message}\nBuilt with uncommitted changes to these files:\n$(git status --porcelain=v1 2>/dev/null | awk '{printf "%s\\n", $0}')"
	fi
else
	version_message="Built outside of git repository; no git version info available\n"
fi
cat << END > "$temp"
#ifndef BUILD_VERSION_H_INCLUDED
#define BUILD_VERSION_H_INCLUDED

#define ARRGEN_VERSION_MESSAGE_GIT "$version_message"

#endif // BUILD_VERSION_H_INCLUDED
END

if ! [[ -e gen_src/build_version_message.h ]] || ! cmp --silent gen_src/build_version_message.h "$temp"; then
    mv "$temp" gen_src/build_version_message.h
else
    rm "$temp"
fi
