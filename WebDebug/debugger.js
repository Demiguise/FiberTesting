var canvas = oCanvas.create({ canvas: "#canvas", background: "#222"})
canvas.settings.fps = 1

var timeToPixel = 5
var curTime = 1

//Fibers

var Fiber = function(id)
{
	console.log("[F:" + id + "] created")
	this.id = id
	this.displayObject = null
}

Fiber.prototype.AddToCanvas = function()
{
	this.displayObject = fiberProto.clone({
		x: (this.id * columnSize) + 5,
		y: curTime * timeToPixel,
		height: curTime * timeToPixel + 1,
		fill: "#33CC66",
	})
	this.displayObject.bind("mouseenter", function(){
		this.fill = fiberColourDim
		canvas.redraw()
	});
	this.displayObject.bind("mouseleave", function(){
		this.fill = fiberColour
		canvas.redraw()
	});
	this.displayObject.name = "Fiber (" + this.id + ")"
	canvas.addChild(this.displayObject)
}

//Threads
var Threads = new Array()
var numThreads = 6

var Thread = function (id)
{
	this.id = id
	this.fibers = []
	console.log("[T:" + id + "] created")
}
Thread.prototype.AddFiberInstance = function(id)
{
	console.log("Adding fiber (" + id + ") to thread (" + this.id + ")")
	var newLen = this.fibers.push(new Fiber(this.id))
	this.fibers[newLen - 1].AddToCanvas()
}

Thread.prototype.Tick = function()
{
	console.log("[T:" + this.id + "] Tick")
	var len = this.fibers.length
	var lastFiber = this.fibers[len - 1]
	if (lastFiber != null)
	{
		lastFiber.displayObject.height = curTime * timeToPixel
	}
}

//Thread/Fiber display information
var columnSize = (canvas.width / numThreads) - 2
var fiberColour = "#33CC66"
var fiberColourDim = "#006600"

var fiberProto = canvas.display.rectangle({
	width: columnSize - 1,
})

//Init thread headers
for (var i = 0 ; i < numThreads ; i++)
{
	Threads.push(new Thread(i))
	var threadDisplay = fiberProto.clone({
		x: (i * columnSize) + 5,
		y: 4,
		height: 30,
		fill: "#fff",
	})
	
	//Add 'n' draw the headers
	canvas.addChild(threadDisplay)
	canvas.addChild(canvas.display.text({
		x: (i * columnSize) + (columnSize / 2),
		y: 17,
		origin: { x: "center", y: "center"},
		font: "bold 12px sans-serif",
		text: "Thread " + i,
		fill: "#0aa"
	}))
}

function DimOtherFibers(fiberId)
{
	
}

Threads[0].AddFiberInstance(0)

console.log("Starting Loop")
canvas.setLoop(function()
{
	for(var i = 0 ; i < numThreads ; ++i)
	{
		Threads[i].Tick()
	}
	curTime++
})

