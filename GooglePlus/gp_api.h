
#ifndef _TA_GOOGLE_PLUS_API_H_
#define _TA_GOOGLE_PLUS_API_H_ 1

#pragma once

static char *GP_URL[kGPUrlCount] = {
			/*Home =*/"https://plus.google.com",
            /*NewPost =*/"https://plus.google.com/%s_/sharebox/post/?spam=23&_reqid=%u&rt=j",
            /*GetActivity =*/"https://plus.google.com/%s_/stream/getactivity/?_reqid=%u&rt=j",
            /*GetActivities =*/"https://plus.google.com/%s_/stream/getactivities/?_reqid=%u&rt=j",
            /*PushInit =*/"https://talkgadget.google.com/talkgadget/channel/test",
            /*PushReceive =*/"https://talkgadget.google.com/talkgadget/channel/bind",
            /*InitImageUpload =*/"https://plus.google.com/%s_/upload/photos/resumable?authuser=0",
            /*UploadImage =*/"https://plus.google.com/%s_/upload/photos/resumable?file_id=000&upload_id={UPLOAD_ID}",
            /*Comment =*/"https://plus.google.com/%s_/stream/comment/?rt=j&_reqid=%u",
            /*PlusOne =*/"https://plus.google.com/%s_/plusone?_reqid=%u&rt=j",
            /*LinkPreview =*/"https://plus.google.com/%s_/sharebox/linkpreview/?c={LINK_LOCATION}&_reqid=%u&rt=j",
            /*GetNotificationsData =*/"https://plus.google.com/%s_/notifications/getnotificationsdata?rt=j&_reqid=%u",
            /*UpdateLastReadTime =*/"https://plus.google.com/%s_/notifications/updatelastreadtime?rt=j&_reqid=%u",
            /*GUC =*/"https://plus.google.com/%s_/n/guc?_reqid=%u&rt=j",
            /*GetProfile =*/"https://plus.google.com/%s_/profiles/get/%u",
            /*EditComment =*/"https://plus.google.com/%s_/stream/editcomment/?_reqid=%u&rt=j",
            /*DeleteComment =*/"https://plus.google.com/%s_/stream/deletecomment/?_reqid=%u&rt=j",
            /*LookupCircles =*/"https://plus.google.com/%s_/socialgraph/lookup/circles/?ct=2&rt=j&m=true&tag=fg&_reqid=%u",
			"https://plus.google.com/%s_/communities/rt/landing?_reqid=%u&rt=j"
};

#endif