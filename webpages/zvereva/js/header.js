const header = document.querySelector("header"),
    icons = document.querySelector(".icons"),
    button = document.querySelector(".button");


const newNav = nav.cloneNode(true),
    newSearch = search.cloneNode(true);

const navbar = document.createElement('section');

navbar.classList.add('new-navbar');

navbar.append(newNav);
navbar.append(newSearch);

header.after(navbar);

button.onclick = () => {
    newNav.classList.remove('none');
    newNav.classList.add('flex');
    newSearch.classList.toggle('flex');
    navbar.classList.toggle('toggle-open');
    button.classList.toggle('button-toggle');
}