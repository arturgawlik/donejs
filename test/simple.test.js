const globbuf = {};
done.syscall.glob("./**/*.test.js", 1 << 3 /* GLOB_DOOFFS */, null, globbuf);
console.log(globbuf.gl_pathc);

for (const gl_pathv of globbuf.gl_pathv) {
    console.log(gl_pathv);
}
