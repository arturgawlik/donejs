hobby attempt to implement V8 based JS runtime

### RUNNING

```bash
$ make
$ ./done ./samples/test.js
```

### Features:

- [x] very basic `console.log`
- [x] es modules
- [ ] es modules dynamic `import()`

### Project improvements

- [ ] improve build system - now for every new c++ module `Makefile` needs to be updated
- [ ] tests infrastructure for sure for js side, but maybe also for cpp side (with [https://github.com/google/googletest](https://github.com/google/googletest))
