// animation object holds numerous properties related to motion
var anime = new Object();

// initialize default anime object
function initAnime() {
    anime = {elemID:"", 
             xStart:0, 
             yStart:0, 
             xCurr:0, 
             yCurr:0, 
             next:1,
             pts:1,
             radius:1,
             interval:null
            };
}

// stuff animation object with necessary explicit and calculated values
function initCircAnime(elemID, startX, startY, pts, radius) {
    initAnime(); 
    anime.elemID = elemID;
    anime.xCurr = anime.xStart = startX;
    anime.yCurr = anime.yStart = startY;
    anime.pts = pts;
    anime.radius = radius;
    // set element's start position
    document.getElementById(elemID).style.left = startX + "px";
    document.getElementById(elemID).style.top = startY + "px";
    // start the repeated invocation of the animation
    anime.interval = setInterval("doCircAnimation()", 10);
}

function doCircAnimation() {
    if (anime.next < anime.pts) {
        var x = anime.xCurr + 
           Math.round(Math.cos(anime.next * (Math.PI/(anime.pts/2))) * anime.radius);
        var y = anime.yCurr + 
           Math.round(Math.sin(anime.next * (Math.PI/(anime.pts/2))) * anime.radius);
        document.getElementById(anime.elemID). style.left = x + "px";
        document.getElementById(anime.elemID). style.top = y + "px";
        anime.xCurr = x;
        anime.yCurr = y;
        anime.next++;
    } else {
        document.getElementById(anime.elemID).style.left = anime.xStart + "px";
        document.getElementById(anime.elemID).style.top = anime.yStart + "px";
        clearInterval(anime.interval);
    }
}

addOnLoadEvent(initAnime);