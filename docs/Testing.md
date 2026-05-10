# Testing

## Linux

From the repository root, configure the Linux preset, build in parallel, and run all enabled tests in parallel with:

```
cmake --preset linux-debug
cmake --build out/build/linux-debug --parallel "$(nproc)"
ctest --test-dir out/build/linux-debug --parallel "$(nproc)" --output-on-failure
```

