// TODO: fix that modules expose not working
import doneJsConsole from "/home/artur-gawlik/git/donejs/lib/console.js";
import exit from "/home/artur-gawlik/git/donejs/lib/process.js";

console = globalThis.done.console;
exit = globalThis.done.exit;
