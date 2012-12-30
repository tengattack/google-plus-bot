
function get_filename(url, addtime) {
	var ifilename = url.lastIndexOf("/");
	if (ifilename > -1) {
		var fn = "";
		if (addtime) {
			fn += (new Date()).valueOf();
			fn += "-";
		}
		fn += url.substr(ifilename + 1);
		return fn;
	} else {
		return false;
	}
}

function is_image_url(url) {
	var filename = get_filename(url, false);
	if (filename !== false) {
		var idot = filename.lastIndexOf(".");
		if (idot > -1) {
			var extname = filename.substr(idot + 1);
			extname = extname.toLowerCase();
			if (extname == "jpg" || extname == "jpeg" ||
				extname == "png" || extname == "gif" || 
				extname == "bmp") {
				return true;
			}
		}
	}
	return false;
}

function create_folder(name, category) {
	var folder = GetCurrentPath() + "community";

	var f = new CFile;
	f.CreateDirectory(folder);

	folder += "/" + name;
	f.CreateDirectory(folder);

	folder += "/" + category;
	f.CreateDirectory(folder);

	folder += "/";

	f.dispose();

	return folder;
}

function gplus_image_url(url) {
	//print(url);
	url = url.replace(/(.*)\/(w[0-9]+\-h[0-9]+\/)(.*?)$/, "$1/$3");
	if (!url.match(/(.*)\/(s0\/)(.*?)$/)) {
		url = url.replace(/(.*)\/(.*?)$/, "$1/s0/$2");
	}
	return url;
}

print(JSON.stringify(post));

if (post.in_community) {

	var type = post.media.type;

	switch (type) {
	case 1: 	//kMTLink
		if (is_image_url(post.media.link_url)) {
			var folder = create_folder(post.community.name, post.community.category);
			var filename = get_filename(post.media.link_url, true);
			if (filename === false) {
				break;
			}
			print("downloading " + post.media.link_url +  "...");
			var succeed = false;
			var file = new CFile;
			if (file.Open(folder + filename, "w")) {
				succeed = file.DownloadFrom(post.media.link_url);
				file.Close();
			}
			file.dispose();
			print("download " + (succeed ? "finish!" : "failed!"));
		}
		break;
	case 2: 	//kMTImage
		{
			var folder = create_folder(post.community.name, post.community.category);
			var count = post.media.images.length;
			for (var i = 0; i < count; i++) {

				var url = gplus_image_url(post.media.images[i]);
				var filename = get_filename(url, true);
				if (filename === false || url === false) {
					continue;
				}
				print("downloading " + url +  "...");
				var succeed = false;
				var file = new CFile;
				if (file.Open(folder + filename, "w")) {
					succeed = file.DownloadFrom(url);
					file.Close();
				}
				file.dispose();
				print("download " + (succeed ? "finish!" : "failed!"));
			}
		}
		break;
	}


}