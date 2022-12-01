// Мировые константы

var pi = 3.141592;
var deg = pi/180;
const worldSize = 1000;
// Конструктор player

function player(x,y,z,rx,ry) {
	this.x = x;
	this.y = y;
	this.z = z;
	this.rx = rx;
	this.ry = ry;
}

// Массив прямоугольников
const map2 = [
    [1, 1, 1, 1],
    [1, 0, 0, 1],
    [1, 0, 0, 1],
    [1, 1, 1, 1],
]


// Нажата ли клавиша и двигается ли мышь?

var PressBack = 0;
var PressForward = 0;
var PressLeft = 0;
var PressRight = 0;
var PressUp = 0;
var MouseX = 0;
var MouseY = 0;

// Введен ли захват мыши?

var lock = false;

// На земле ли игрок?

var onGround = true;

// Привяжем новую переменную к container

var container = document.getElementById("container");

// Обработчик изменения состояния захвата курсора

document.addEventListener("pointerlockchange", (event)=>{
	lock = !lock;
});

// Обработчик захвата курсора мыши

container.onclick = function(){
	if (!lock) container.requestPointerLock();
};

// Обработчик нажатия клавиш

document.addEventListener("keydown", (event) =>{
	if (event.key == "a"){
		PressLeft = 1;
	}
	if (event.key == "w"){
		PressForward = 1;
	}
	if (event.key == "d"){
		PressRight = 1;
	}
	if (event.key == "s"){
		PressBack = 1;
	}
	if (event.keyCode == 32 && onGround){
		PressUp = 1;
	}
});

// Обработчик отжатия клавиш

document.addEventListener("keyup", (event) =>{
	if (event.key == "a"){
		PressLeft = 0;
	}
	if (event.key == "w"){
		PressForward = 0;
	}
	if (event.key == "d"){
		PressRight = 0;
	}
	if (event.key == "s"){
		PressBack = 0;
	}
	if (event.keyCode == 32){
		PressUp = 0;
	}
});

// Обработчик движения мыши

document.addEventListener("mousemove", (event)=>{
	MouseX = event.movementX;
	MouseY = event.movementY;
});

// Создаем новый объект

var pawn = new player(0,0,0,0,0);

// Привяжем новую переменную к world

var world = document.getElementById("world");

function update(){
	
	// Задаем локальные переменные смещения
	
	let dx =   (PressRight - PressLeft)*Math.cos(pawn.ry*deg) - (PressForward - PressBack)*Math.sin(pawn.ry*deg);
	let dz = - (PressForward - PressBack)*Math.cos(pawn.ry*deg) - (PressRight - PressLeft)*Math.sin(pawn.ry*deg);
	let dy = - PressUp;
	let drx = MouseY;
	let dry = - MouseX;
	
	// Обнулим смещения мыши:
	
	MouseX = MouseY = 0;
	
	// Прибавляем смещения к координатам
	
	pawn.x = pawn.x + dx;
	pawn.y = pawn.y + dy;
	pawn.z = pawn.z + dz;
	
	// Если курсор захвачен, разрешаем вращение
	
	if (lock){
		pawn.rx = pawn.rx + drx;
		pawn.ry = pawn.ry + dry;
	};

	// Изменяем координаты мира (для отображения)
	
	world.style.transform = 
	"translateZ(" + (600 - 0) + "px)" +
	"rotateX(" + (-pawn.rx) + "deg)" +
	"rotateY(" + (-pawn.ry) + "deg)" +
	"translate3d(" + (-pawn.x) + "px," + (-pawn.y) + "px," + (-pawn.z) + "px)";
	
};
let map = [
    [0,0,1000,0,180,0,2000,200,"#F0C0FF"],
  /*[0,0,-1000,0,0,0,2000,200,"#F0C0FF"],
    [1000,0,0,0,-90,0,2000,200,"#F0C0FF"],
    [-1000,0,0,0,90,0,2000,200,"#F0C0FF"],
    [0,0,-300,70,0,0,200,500,"#F000FF"],
    [0,-86,-786,90,0,0,200,500,"#F000FF"],
    [-500,0,-300,20,0,0,200,500,"#00FF00"],*/
    [0,100,0,90,0,0,2000,2000,"#666666"]
];
let walls = [
    [0,100,0,90,0,0,2000,2000,"#666666"]
];
let wx,wy,wz,wax,way,waz,ww,wh;
let buildWall = false;
function CreateNewWorld(){
    for (let i = 0; i < map2.length; i++) {
        for (let j = 0; j < map2[i].length; j++) {
            console.log(map2[i][j]);
            if (map2[i][j] == 1) {
                if (buildWall){
                    
                }
                else {
                    buildWall = true;
                    wx = i * 250;
                    wy = 0;
                    wz = j * 250;
                    wax = 0;
                    way = 0;
                    waz = 0;
                    wh = 200;
                }
            }
            if (j+1 == map2[i].length){
                if (buildWall){
                    ww = 250*(j+1) - wx;
                    buildWall = false; 
                    walls.push([wx,wy,wz,wax,way,waz,ww,wh,"#F0C0FF"]);
                }
            }
                
        }
    }
    console.log(walls);
	for (let i = 0; i < walls.length; i++){
		
		// Создание прямоугольника и придание ему стилей
		
		let newElement = document.createElement("div");
		newElement.className = "square";
		newElement.id = "square" + i;
		newElement.style.width = walls[i][6] + "px";
		newElement.style.height = walls[i][7] + "px";
		newElement.style.background = walls[i][8];
		newElement.style.transform = "translate3d(" +
		(600 - walls[i][6]/2 + walls[i][0]) + "px," +
		(400 - walls[i][7]/2 + walls[i][1]) + "px," +
		                    (walls[i][2]) + "px)" +
		"rotateX(" + walls[i][3] + "deg)" +
		"rotateY(" + walls[i][4] + "deg)" +
		"rotateZ(" + walls[i][5] + "deg)";
		
		// Вставка прямоугольника в world
		
		world.append(newElement);
	}
}

CreateNewWorld();
TimerGame = setInterval(update,10);