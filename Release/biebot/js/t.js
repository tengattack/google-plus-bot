

function htmlesc(strvalue) {
	strvalue = strvalue.replace(/<.*?>/g,"");
	strvalue = strvalue.replace(/&amp;/g,"&");
	strvalue = strvalue.replace(/&lt;/g,"<");
	strvalue = strvalue.replace(/&gt;/g,">");
	strvalue = strvalue.replace(/&quot;/g,"\"");
	strvalue = strvalue.replace(/&#39;/g,"'");
	return strvalue;
}

print(baseinfo.content);
//print(baseinfo.post_id);

//check mention
if (baseinfo.user != "") {

	if (baseinfo.content.indexOf(">" + baseinfo.user + "<") != -1) {

		var getreply = false;
		var regDice = /([0-9]+)[D|d]([0-9.]+)/;
		var f = baseinfo.content.match(regDice);
		if (f) {
			var n1 = Number(f[1]);
			var n2 = Number(f[2]);
			var isfloat = (f[2].indexOf(".") != -1);
			if (n1 && n2) {
				var text = "@" + baseinfo.author_id + " *" + n1.toString() + "* 个骰子骨碌骨碌转, 出现了 ";
				for (var i = 0; i < n1; i++) {
					var tn = Math.random() * n2;
					if (isfloat) {
						text += "*" + tn.toFixed(2) + "* ";
					} else {
						text += "*" + Math.floor(tn + 1) + "* ";
					}
				}
				text += "\n*别* 欺负人家啦, 好累啊!";

				print("gplus.Comment... " + text);
				if (gplus.Comment(baseinfo.post_id, text)) {
					print("gplus.Comment succeed!");
				} else {
					print("gplus.Comment failed!");
				}
				getreply = true;
			}
		} else {
			var regExec = /\[\?v8\s(.*)\?\]/;
			var f = baseinfo.content.match(regExec);
			if (f && f[1]) {
				var command = htmlesc(f[1]);
				print(command);

				var d = eval(command);
				getreply = true;

				print("gplus.Comment... " + d);
				gplus.Comment(baseinfo.post_id, "@" + baseinfo.author_id + " " + d + "\n*别* 这样捉弄人家嘛~");
			}
		}

		if (!getreply) {
			var reply_text = "";
		    //var strvalue = baseinfo.content.replace(/<.*?>/g, " ");
		    var startvalue = baseinfo.content.indexOf("\u00a0");
		    if (startvalue == -1) {
		    	startvalue = baseinfo.content.indexOf(" ");
		    }
		    if (startvalue != -1) {
			    var svalue = baseinfo.content.substr(startvalue + 1);

			    svalue = svalue.replace(/<.*?>/g, " ");
			    var regQ = /(\s*)?(.*)(\s*)?/;
			    var f = svalue.match(regQ);

			    if (f && f[2]) {

			        var question = htmlesc(f[2]);

			        if (question && question.indexOf("'") == -1) {

				        reply_text = "*" + question + "* 是什么? 快教教人家吧! "; 
				        var dbpath = GetCurrentPath() + 'db/qa.db';
				        var sqlite = new CSqlite;

				        if (sqlite.Open(dbpath)) {

				            print("sqlite.Open succeed!");
				            if (sqlite.Query("SELECT * FROM qa_db WHERE question Like '%" + question + "%';")) {
				                
				                var result = sqlite.GetResult();
				                if (result && result.length && result.length > 0) {
				                	var index = Math.floor(Math.random() * result.length);
					                reply_text = result[index].answer;
					                getreply = true;
					            }
				            } else {
				                print("sqlite.Query SELECT failed! " + sqlite.GetErrorMessage());
				                reply_text = sqlite.GetErrorMessage();
				            }
				        }
				        sqlite.dispose();
				    } else {
				    	reply_text = "*" + question + "* 有点奇怪呢! "; 
				    }
			    }
			}

	        
	        if (!getreply && !reply_text) {
	        	reply_text = "别这样嘛~";
	        }
	        print("gplus.Comment... " + reply_text);
	        if (gplus.Comment(baseinfo.post_id, "@" + baseinfo.author_id + " " + reply_text)) {
	        	print("succeed!");
	        } else {
	        	print("failed!");
	        }
		}

	} else {

		print("Ignore.");
	}
}
 
