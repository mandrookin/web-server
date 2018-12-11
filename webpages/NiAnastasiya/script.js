const anchors = document.querySelectorAll('a');
const ico = document.querySelectorAll('img.ico');
const slider__one =  document.querySelectorAll('.slider__one');
const slider__two =  document.querySelectorAll('.slider__two');
const radioOne =  document.querySelector('.radioOne');
const radioTwo =  document.querySelector('.radioTwo');


radioOne.addEventListener('click', function(){
  for(let i = 0; i < slider__one.length; i++){
    slider__two[i].classList.remove("visible");
    slider__two[i].classList.add("invisible");
    slider__one[i].classList.add("visible");
    slider__one[i].classList.remove("invisible");
    radioOne.style.backgroundColor = '#333333';
    radioTwo.style.backgroundColor = '#656565';
  }
});

radioTwo.addEventListener('click', function(){
  for(let i = 0; i < slider__two.length; i++){
    slider__one[i].classList.remove("visible");
    slider__one[i].classList.add("invisible");
    slider__two[i].classList.add("visible");
    slider__two[i].classList.remove("invisible");
    radioOne.style.backgroundColor = '#656565';
    radioTwo.style.backgroundColor = '#333333';
    
  }
});


for(let i = 0; i < ico.length; i++){
  ico[i].onmouseover = function(event){
    var target = event.target;
    target.style.backgroundColor = '#333333';
  }
}    

for(let i = 0; i < ico.length; i++){
  ico[i].onmouseout = function(event){
    var target = event.target;
    target.style.backgroundColor = '#656565';
  }
}  

anchors[0].onclick = function(e){
  e.preventDefault();
  window.location.hash = "Home";
}; 
anchors[1].onclick = function(e){
  e.preventDefault();
  window.location.hash = "About";
}; 
anchors[2].onclick = function(e){
  e.preventDefault();
  window.location.hash = "Portfolio_Showcase2";
}; 
anchors[3].onclick = function(e){
  e.preventDefault();
  window.location.hash = "Contact";
};