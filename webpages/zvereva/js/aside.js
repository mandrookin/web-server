const form = document.querySelector('.form'),
    about = document.querySelector('.about'),
    aside = document.querySelector('aside'),
    posts = document.querySelector('.posts'),
    search = document.querySelector(".search"),
    nav = document.querySelector("nav");

if (window.outerWidth < '770') {
    posts.before(form);
    aside.remove();
}

document.querySelector('body').onresize = () => {
    if (window.outerWidth < '770') {
        posts.before(form);
        aside.remove();
    } else {
        posts.after(aside);
        about.after(form);
    }
}