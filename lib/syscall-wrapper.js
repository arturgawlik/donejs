const { syscall } = globalThis.done;

// if something can be in js land it should be
Object.defineProperty(syscall, 'macros', {
    value: {
        WEXITSTATUS
    }
});

/**
 * This is copied macro from c++ to js
 * @param {*} status from wait/waitpid syscalls 
 * @returns Real exit code from `status`
 */
function WEXITSTATUS(status) {
    return (((status) & 0xff00) >> 8);
}

export default syscall;
