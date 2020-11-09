const pages = [
    { name: "Home", file: "index.html" },
    { name: "Installation", file: "install.html" },
    { name: "Basic Usage", file: "usage.html" },
    { name: "Documentation", file: "docs.html" },
    { name: "About", file: "about.html" }
];

pages.forEach(page => {
    const but = document.createElement("button");
    but.innerHTML = page.name;
    but.classList.add("nav-button");
    if (window.location.href.endsWith(page.file)) but.classList.add("nav-selected");
    but.setAttribute("onclick", `window.location.href = "${page.file}"`);
    document.getElementById("main-nav").appendChild(but);

    if (document.querySelector("#page-nav") != null) {
        const li = document.createElement("li");
        li.innerHTML = `<a href="${page.file}">${page.name}</a>`;
        document.querySelector("#page-nav").appendChild(li);
    }
});

const navtog = document.createElement("button");
navtog.innerHTML = "≡";
navtog.classList.add("nav-toggle");
navtog.addEventListener("click", e => {
    const n = document.getElementsByTagName("nav")[0];
    n.hasAttribute("off") ? n.removeAttribute("off") : n.setAttribute("off", "");
});
document.getElementsByTagName("header")[0].insertBefore(navtog, document.querySelector("header text"));

document.getElementById("main-nav").innerHTML += "<footer><text>GDit © HJfod 2020.</text></footer>";

if (document.querySelector("#license-text") != null)
    fetch("https://raw.githubusercontent.com/HJfod/gdit/master/LICENSE").then(res => res.text().then(t =>
        document.querySelector("#license-text").innerHTML = t.replace(/\n/g, "<br>")));

document.querySelectorAll("table-from").forEach(t => {
    const j = JSON.parse(t.getAttribute("from"));

    const ta = document.createElement("table");
    for (let i = 0; i < j[0].length; i++) {
        const tr = document.createElement("td");
        j.forEach(jj => {
            const td = document.createElement("tr");
            td.innerHTML = jj[i];
            tr.appendChild(td);
        });
        ta.appendChild(tr);
    }

    t.parentNode.insertBefore(ta, t);

    t.remove();
});