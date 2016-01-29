// https://github.com/pebble-examples/slate-config-example/blob/master/src/js/pebble-js-app.js

var timeout=null;

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  //localStorage.removeItem("settings"); localStorage.removeItem("username");  console.warn("Overwriting data saved on phone!!!");
  var settings=localStorage.getItem("settings");
  send_dict_to_watch(settings ? JSON.parse(settings) : {});
  server_checkdata();
});

function send_dict_to_watch(dict) {
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
}

Pebble.addEventListener("appmessage",	function(e) {
	console.log("Received message: " + JSON.stringify(e.payload));
  if (e.payload.KEY_EXPORT) {
    delete e.payload.KEY_EXPORT;
    localStorage.setItem("settings",JSON.stringify(e.payload));
    showConfiguration();
  } else {
    localStorage.setItem("settings",JSON.stringify(e.payload));
  }
  server_checkdata();
});

function server_checkdata() {
  if (timeout) clearTimeout(timeout);
  timeout=null;
  var username=JSON.parse(localStorage.getItem("username"));
  if (!username) return;
  
  var req = new XMLHttpRequest();
  console.log(JSON.stringify(username));
  req.open('POST', 'https://www.nzmodels.com/pebble/sticker-chart-config/ajax.php'+
           '?func=get_children'+
           '&username='+encodeURIComponent(username.username)+
           '&password='+encodeURIComponent(username.password),
           true);
  req.onload = function(e) {
    if (req.readyState != 4 || req.status != 200) return console.log('Error');
    if (req.status != 200) return console.log('Error 2');
    
    console.log('AJAX Resonse: ' + req.responseText);
    var ajax=JSON.parse(req.responseText);
    if (!ajax.success) return console.log("server error: "+req.responseText);
    
    ajax=ajax.success;
    var dict=JSON.parse(localStorage.getItem("settings"));
    ajax.timestamp = ajax.timestamp * 1; // force to integer from string;
    if (!dict || ajax.timestamp > dict.KEY_TIMESTAMP ) {
      console.log("server data is newer, sending to watch...");
      server_send_server_to_watch(ajax, dict);
    } else if (ajax.timestamp < dict.KEY_TIMESTAMP) {
      console.log("server data is older, sending to server...");
      server_send_watch_to_server(username, dict);
    } else {
      console.log("data is already up-do-date!");
    }
  };
  req.send(null);
  timeout=setTimeout(server_checkdata,30*1000); // 30 seconds
}

function server_send_watch_to_server(username, dict) {
  var req = new XMLHttpRequest();
  var children = [];
  var child=0;
  while (child===0 ? dict.KEY_CHILDREN : dict[100+child]) {
    children.push(child===0 ? dict.KEY_CHILDREN : dict[100+child]);
    child++;
  }
  req.open('POST', 'https://www.nzmodels.com/pebble/sticker-chart-config/ajax.php'+
           '?func=update_children'+
           '&username='+encodeURIComponent(username.username)+
           '&password='+encodeURIComponent(username.password)+
           '&children='+encodeURIComponent(JSON.stringify(children))+
           '&timestamp='+encodeURIComponent(dict.KEY_TIMESTAMP),
            true);
  req.onload = function(e) {
    if (req.readyState != 4 || req.status != 200) return console.log('Error');
    if (req.status != 200) return console.log('Error 2');
    
    console.log('AJAX Resonse: ' + req.responseText);
    var ajax=JSON.parse(req.responseText);
    if (!ajax.success) return console.log("server error: "+req.responseText);
    send_dict_to_watch(dict);
  };
  req.send(null);
}

function server_send_server_to_watch(ajax, dict) {
  
  //console.log("ajax: "+JSON.stringify(ajax));
  //console.log("dict: "+JSON.stringify(dict));
  
  // ajax: {"children":"[\"Millie? =&|2| H\",\"Penny|2|4 \"]","timestamp":1453568340}
  // dict: {"101":"Penny|2|4 ","KEY_CHILDREN":"Millie|2| H","KEY_TIMESTAMP":145356834,"KEY_APP_VERSION":"1.0","KEY_VERSION":1}
  ajax.children=JSON.parse(ajax.children);
  ajax.data_version = dict.KEY_VERSION;
  send_config_to_watch(ajax);
}

function send_config_to_watch(configData) {
  var dict = {};
  dict.KEY_CONFIG_DATA  = 1;
  dict.KEY_VERSION = configData.data_version;
  dict.KEY_TIMESTAMP = configData.timestamp*1.0;
  var children=configData.children;
  var child=0;
  while (children[child]) {
    dict[100+child]=children[child];
    child++;
  }
  
  
  if (configData.username) {
    localStorage.setItem("username",JSON.stringify({username: configData.username, password: configData.password}));
  } else {
    localStorage.removeItem("username");
  }

  // Send to watchapp
  send_dict_to_watch(dict);
}

Pebble.addEventListener('showConfiguration', showConfiguration);

function showConfiguration() {
  var url = 'https://www.nzmodels.com/pebble/sticker-chart-config/index.html';
  var settings=localStorage.getItem("settings");
  var config="";
  if (settings) {
    settings=JSON.parse(settings);
    config+="?app_version="+(settings.KEY_APP_VERSION ? settings.KEY_APP_VERSION : "1.1");
    config+="&data_version="+settings.KEY_VERSION;
    config+="&timestamp="+settings.KEY_TIMESTAMP;
    
    var children = [];
    var child=0;
    while (child===0 ? settings.KEY_CHILDREN : settings[100+child]) {
      children.push(child===0 ? settings.KEY_CHILDREN : settings[100+child]);
      child++;
    }
    config+="&children="+encodeURIComponent(JSON.stringify(children));
  }
  var username=JSON.parse(localStorage.getItem("username"));
  if (username) config+="&username="+username.username+"&password="+username.password;

  console.log('Showing configuration page: ' + config);

  Pebble.openURL(url+config);
}

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  send_config_to_watch(configData);
});