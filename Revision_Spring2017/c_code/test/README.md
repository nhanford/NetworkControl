
To run a test do
```
> ./run.sh TEST DEST
```

Where TEST is a name like `foo` or `bar/foo`. DEST is a destination server such
as `denv-pt1.es.net`. See [a list of
servers](https://fasterdata.es.net/performance-testing/perfsonar/esnet-perfsonar-services/esnet-iperf-hosts/).

To plot the data from a test run
```
> python plot.py TEST [--title TITLE] [--output FILE]
```

Note that the times from BWCTL and the kernel modules don't line up.
