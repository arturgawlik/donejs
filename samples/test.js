try {
    const res = fetch("http://localhost:5001");
    console.log(`recv data decoded: ${res}`);

} catch (e) {
    console.log(e.message);
}
