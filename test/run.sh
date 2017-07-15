#!/bin/sh

cd $(dirname $0)/..

for test in $(find test -type f); do
	if [ -x $test ]; then
		$test 2>/dev/null || echo "$test failed" >&2
	fi
done
