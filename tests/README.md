# Conversion regression checks

These checks run the production Clash parser and Surge exporter with synthetic
nodes. They need a C++20 compiler, CMake, yaml-cpp, toml11, RapidJSON and PCRE2.
They do not contact a proxy server or verify network connectivity.

```sh
cmake -S tests -B build-tests
cmake --build build-tests --parallel
ctest --test-dir build-tests --output-on-failure
```

For dependencies outside standard locations, pass `CMAKE_PREFIX_PATH`,
`RAPIDJSON_INCLUDEDIR`, and `PCRE2_ROOT` to CMake as needed.
