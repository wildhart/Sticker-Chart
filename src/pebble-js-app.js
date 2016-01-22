// https://github.com/pebble-examples/slate-config-example/blob/master/src/js/pebble-js-app.js

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  var settings=localStorage.getItem("settings");
  //settings='{}';
  var dict=settings ? JSON.parse(settings) : {};
  if (!dict.KEY_TIMESTAMP) { 
    var d=new Date();
    dict.KEY_TIMESTAMP = Math.floor(d.getTime()/1000 - d.getTimezoneOffset()*60);
  }
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

Pebble.addEventListener("appmessage",	function(e) {
	console.log("Received message: " + JSON.stringify(e.payload));
  if (e.payload.KEY_EXPORT) {
    delete e.payload.KEY_EXPORT;
    localStorage.setItem("settings",JSON.stringify(e.payload));
    showConfiguration();
  } else {
    localStorage.setItem("settings",JSON.stringify(e.payload));
  }
});

Pebble.addEventListener('showConfiguration', showConfiguration);

function padZero(i) {
  return (i<10 ? "0":"")+i;
}

function showConfiguration() {
  var url = 'https://wildhart.github.io/sticker-chart-config/index.html';
  var settings=localStorage.getItem("settings");
  var config="";
  if (settings) {
    settings=JSON.parse(settings);
    config+="?app_version="+(settings.KEY_APP_VERSION ? settings.KEY_APP_VERSION : "1.1");
    config+="&data_version="+settings.KEY_VERSION;
    
    var child=0;
    while (child===0 ? settings.KEY_CHILDREN : settings[100+child]) {
      var data=(child===0 ? settings.KEY_CHILDREN : settings[100+child]).split("|");
      config+="&child_"+child+"="+encodeURIComponent(data.join("|"));
      child++;
    }
  }
  console.log('Showing configuration page: ' + url + config);

  Pebble.openURL(url+config);
}

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  var dict = {};
  dict.KEY_CONFIG_DATA  = 1;
  dict.KEY_VERSION = configData.data_version;
  var d=new Date();
  dict.KEY_TIMESTAMP = Math.floor(d.getTime()/1000 - d.getTimezoneOffset()*60);
  
  var child=0;
  while (configData["child_"+child]) {
    dict[100+child]=decodeURIComponent(configData["child_"+child]);
    child++;
  }

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});