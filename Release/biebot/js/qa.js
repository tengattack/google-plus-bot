

function htmlesc(strvalue) {
    strvalue = strvalue.replace(/<.*?>/g, "");
	strvalue = strvalue.replace(/&amp;/g,"&");
	strvalue = strvalue.replace(/&lt;/g,"<");
	strvalue = strvalue.replace(/&gt;/g,">");
	strvalue = strvalue.replace(/&quot;/g,"\"");
	strvalue = strvalue.replace(/&#39;/g,"'");
	return strvalue;
}

//check mention
if (baseinfo.user != "") {

	if (baseinfo.content.indexOf(">" + baseinfo.user + "<") != -1) {

	    var getreply = false;
	    var reply_text = "别这样嘛~";
		var regQ = /[q|Q]\((.+?)\)/;
		var regA = /[a|A]\((.+?)\)/;
		var regF = /[f|F]\((.+?)\)/;
		var regD = /[d|D]\(([0-9]+?)\)/;
		var fQ = baseinfo.content.match(regQ);
		var fA = baseinfo.content.match(regA);
		var fF = baseinfo.content.match(regF);
		var fD = baseinfo.content.match(regD);
		var islist = (baseinfo.content.indexOf("list") > -1);

		if ((fQ && fQ[1]) || (fD && fD[1])) {
			var question, answer, del, delid;
			if (fQ && fQ[1]) {
				question = htmlesc(fQ[1]);
			}
			if (fA && fA[1]) {
				answer = htmlesc(fA[1]);
			}
			if (fF && fF[1]) {
				del = htmlesc(fF[1]);
			}
			if (fD && fD[1]) {
				delid = Number(htmlesc(fD[1]));
			}

		    //check sql
		    if (question && question.indexOf("'") != -1) {
		    	question = "";
		    }
		    if (answer && answer.indexOf("'") != -1) {
		    	answer = "";
		    }
		    if (del && del.indexOf("'") != -1) {
		    	del = "";
		    }

		    if ((question && (answer || del || islist)) || delid) {
		        var dbpath = GetCurrentPath() + 'db/qa.db';
		        var sqlite = new CSqlite;
		        if (sqlite.Open(dbpath)) {
		            print("sqlite.Open succeed!");
		            if (sqlite.Query('CREATE TABLE qa_db (id INTEGER PRIMARY KEY AUTOINCREMENT,' +
		                 ' question char(255) NOT NULL DEFAULT \'\',' +
		                 ' answer char(255) NOT NULL DEFAULT \'\'' +
		                 ');')) {
		                print("sqlite.Query CREATE TABLE succeed!");
		            } else {
		                print("sqlite.Query CREATE TABLE failed! " + sqlite.GetErrorMessage());
		            }

		            reply_text = "";
		            if (delid) {

						if (sqlite.Query("DELETE FROM qa_db WHERE id = '" + delid + "';")) {
			                print("sqlite.Query DELETE succeed! " + delid.toString());
			                getreply = true;

			                reply_text += "えい, 人家好像忘记了什么..." + delid.toString() + "...";
			            } else {
			                print("sqlite.Query DELETE failed! " + sqlite.GetErrorMessage());
			                reply_text += "" + sqlite.GetErrorMessage();
			            }
			        }
			        if (question) {
			            if (answer) {
			            	if (delid) reply_text += "\n";

				            if (sqlite.Query("INSERT INTO qa_db (question, answer) values('" + question + "', '" + answer + "');")) {
				                print("sqlite.Query INSERT succeed! " + question + " : " + answer);
				                getreply = true;

				                reply_text = "噢噢, 人家记住了呢! " + question + " : " + answer;
				            } else {
				                print("sqlite.Query INSERT failed! " + sqlite.GetErrorMessage());
				                reply_text = sqlite.GetErrorMessage();
				            }
				        }
				        if (del) {
				        	if (answer || delid) reply_text += "\n";

							if (sqlite.Query("DELETE FROM qa_db WHERE question LIKE '%" + question + "%' AND answer LIKE '%" + del + "%';")) {
				                print("sqlite.Query DELETE succeed! " + question + " : " + del);
				                getreply = true;

				                reply_text += "えい, 人家好像忘记了什么..." + question + "...";
				            } else {
				                print("sqlite.Query DELETE failed! " + sqlite.GetErrorMessage());
				                reply_text += "" + sqlite.GetErrorMessage();
				            }
				        }
				        
				        if (islist) {
				        	if (answer || del || delid) reply_text += "\n";
				        	if (sqlite.Query("SELECT * FROM qa_db WHERE question Like '%" + question + "%';")) {
				                print("sqlite.Query SELECT succeed! " + question);
				                getreply = true;

				                var result = sqlite.GetResult();
				                reply_text += JSON.stringify(result);
				            } else {
				                print("sqlite.Query SELECT failed! " + sqlite.GetErrorMessage());
				                reply_text += "" + sqlite.GetErrorMessage();
				            }
				        }
				    }
		        }
		        sqlite.dispose();
		    }
		}
		print("gplus.Comment... " + reply_text);
		if (gplus.Comment(baseinfo.post_id, "@" + baseinfo.author_id + " " + reply_text + "\n*别* 这样玩人家啦~")) {
			print("succeed!");
		} else {
			print("failed!");
		}
		
	} else {

		print("Ignore.");
	}
}
 
print("");