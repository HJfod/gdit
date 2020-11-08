const pages = [
    { name: "Home", file: "index.html" },
    { name: "Installation", file: "install.html" },
    { name: "Basic Usage", file: "usage.html" },
    { name: "About", file: "about.html" }
];

pages.forEach(page => {
    const but = document.createElement("button");
    but.innerHTML = page.name;
    but.classList.add("nav-button");
    if (window.location.href.endsWith(page.file)) but.classList.add("nav-selected");
    but.setAttribute("onclick", `window.location.href = "${page.file}"`);
    document.getElementById("main-nav").appendChild(but);
});