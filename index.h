const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html> <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no" />
    <title>Spectral Landscapes Control</title>
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
      body{margin-top: 0px;} h1 {color: #444444;margin: 0px auto 0px;} h3 {color: #444444;margin-bottom: 0px;}
      .button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px; }
      .button-on {background-color: #3498db;}\n";
      .button-on:active {background-color: #2980b9;}
      .button-off {background-color: #34495e;}
      .button-off:active {background-color: #2c3e50;}
      p {font-size: 14px;color: #888;margin-bottom: 0px; }
      .card{ max-width: px; min-height: 0px; background: #02b875; padding: 10px; box-sizing: border-box; color: #FFF; margin:20px; box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);}
    </style>
 </head>
<body>
  <h1>SPECTRAL LANDSCAPES</h1>
  <a href="/edit">GO TO FILESYSTEM</a>
  <br>

  <a href="/">RELOAD PAGE</a>
  <br>
  <div class="card"> <h1>CPS <span id="CPSValue">0</span></h1> </div>
  <script> setInterval(function() {  getData1(); }, 1000); function getData1() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {document.getElementById("CPSValue").innerHTML =  this.responseText; } }; xhttp.open("GET", "readCPS", true); xhttp.send(); } </script>

  <div class="card"> <h1>GPSfix <span id="GPSfix">0</span></h1> </div>
  <script> setInterval(function() {  getData2(); }, 1500); function getData2() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {document.getElementById("GPSfix").innerHTML =  this.responseText; } }; xhttp.open("GET", "readGPSfix", true); xhttp.send(); } </script>
 
  <div class="card"> <h1>Battery level <span id="Battery">0</span></h1> </div>
  <script> setInterval(function() {  getData3(); }, 12400); function getData3() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {document.getElementById("Battery").innerHTML =  this.responseText; } }; xhttp.open("GET", "readBattery", true); xhttp.send(); } </script>

  <div class="card"> <h1>FS Space <span id="Space">0</span></h1> </div>
  <script> setInterval(function() {  getData4(); }, 10300); function getData4() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {document.getElementById("Space").innerHTML =  this.responseText; } }; xhttp.open("GET", "readFSspace", true); xhttp.send(); } </script>
</body>
</html>
  
)=====";
