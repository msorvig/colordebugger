// Qt Progressive Enhancement Loader
//
// The Qt progressive enhancement loader supports progressive loading
// of Qt for WebAssembly applications. This is accomplished by replacing basic
// content (such as an image) with the Qt application canvas if and when the
// user "clicks trough". The basic content is left on the page if WebAssembly
// is unsupported, or if script is disabled or blocked, or if the user does 
// not activate the application.
//
// The PE loader expects a spesific html and application file structure,
// which is identified by the application name ("TestApp" in the examples
// below.)
//
// Javascript Usage:
// 
//  var loader = QtProgressiveEnhancementLoader({
//.     name : "TestApp"
//  });
//  loader.load();
// 
// HTML structure:
//   <div id = "qt-testapp-container">
//     <img id = "qt-testapp-basic" onclicked = "loader.load()"> </img>     
//   </div>
//
// Application file structure:
//   testapp.js     (with TestApp() Emscripten module constructor function).
//   testapp.wasm
//   qtloader.js
// (This matches the output produced by the Qt build system)
//
// Note that the config object also accepts QtLoader config options.
//

function QtProgressiveEnhancementLoader(config)
{
    var name = config.name.toLowerCase();
    var path = config.path || ""

    var jsLoadState = "NotLoaded";
    function initiateLoad() {
        console.log(name)
        
        // Assume non-support for fetch means no Wasm support either.
        if (fetch === undefined)
            return;

        // Handle has-loaded/is-loading/failed states
        if (jsLoadState == "Loaded") {
            completeLoad();
            return;
        }
        if (jsLoadState == "Loading")
            return;
        if (jsLoadState == "Failed")
            return;

        jsLoadState == "Loading";
        
        // Fetch and eval app loader scripts
        function evalScript(url) {
            return new Promise((resolve, reject) => {
                fetch(new Request(url)).then(function(response) {
                    if (!response.ok) {
                        reject(url + " " + response.statusText)
                    } else {
                        response.text().then(function(text) {
                            this.eval(text); // ES5 indirect global scope eval
                            resolve();
                        });
                    }
                });
            });
        }
        
        var scripts = [ evalScript(`${path}qtloader.js`), 
                        evalScript(`${path}${name}.js`) ];
        Promise.all(scripts).then(function(){
            console.log(QtLoader.name);
            console.log(Module.name);
            completeLoad();
        }).catch(function(error) {
            jsLoadState = "Failed"
            console.log("Script error:" + error);
        });
    }

    function completeLoad() {
        console.log("completeLoad " + this);
        console.log(QtLoader.name)
        
        // Configure and create QtLoader
        
        config.restartMode = "RestartOnExit"
    
        // Less chatty
        config.stdoutEnabled = false;
        config.stderrEnabled = true;
    
        // Prepare for juggling basic and canvas content based on load status.
        var containerId = `qt-${name}-container`;
        var container = document.getElementById(containerId);
        var basicContentId = `qt-${name}-basic`;
        var basicContent = document.getElementById(basicContentId);
        var canvas = document.createElement("canvas");
        canvas.style.display = "none";
        canvas.width = "100%";
        canvas.height = "100%";
        container.appendChild(canvas);

        config.showLoader = function(container) {
            basicContent.style.display = "block"
            canvas.style.display = "none";
        }

        config.showCanvas = function(container) {
            basicContent.style.display = "none"
            canvas.style.display = "block";
            return canvas;
        }

        config.showExit = function(container) {
            basicContent.style.display = "block"
            canvas.style.display = "none";
        }
    
        config.showError = function(container, errorText) {
            basicContent.style.display = "block"
            canvas.style.display = "none";
        }
        
        var qtLoader = QtLoader(config);
        
        // Silenty fail if the browser does not support Wasm or WebGL
        if (!qtLoader.canLoadApplication)
            return;

        // Fail and complain if the HTML structure is incorrect
        if (container == null) {
            console.log(`QtProgressiveEnhancementLoader error: Content container with id ${containerId} not found`);
            return;
        }
        if (basicContent == null) {
            console.log(`QtProgressiveEnhancementLoader error: Basic content with id ${basicContentId} not found`);
            return;
        }
        
        qtLoader.loadEmscriptenModule(Module);
    }

    var publicAPI = {
        "load" : initiateLoad
    }
    return publicAPI;
}

