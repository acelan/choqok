/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2011 AceLan Kao <acelan@acelan.idv.tw>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see http://www.gnu.org/licenses/
*/
#ifndef PLURKAPI_H
#define PLURKAPI_H

#include <QtCore/QDateTime>
#include <QStringList>

const QString plurkApiUrl= "http://www.plurk.com/APP/";

/* 
   Depending on what kind of request it is the data returned varies. For responses and plurks, the data returned is minimal and will look like this:
   {"display_name": "amix3", "gender": 0, "nick_name": "amix", "has_profile_image": 1, "id": 1, "avatar": null}

   This can be used to render minimal info about a user.
   For other type of requests, such as viewing a friend list or a profile, the data returned will be larger:

   {"display_name": "Alexey", "is_channel": 0, "nick_name": "Scoundrel", "has_profile_image": 1, "location": "Canada", "date_of_birth": "Sat, 19 Mar 1983 00:00:00 GMT", "relationship": "not_saying", "avatar": 3, "full_name": "Alexey Kovyrin", "gender": 1, "page_title": "", "recruited": 6, "id": 5, "karma": 33.5}
 */

/*
   How to render the avatar
   One needs to construct the avatar URL. user_id specifies user's id while avatar specifies the profile image version.
   If has_profile_image == 1 and avatar == null then the avatar is:

http://avatars.plurk.com/{user_id}-small.gif 
http://avatars.plurk.com/{user_id}-medium.gif 
http://avatars.plurk.com/{user_id}-big.jpg

If has_profile_image == 1 and avatar != null:
http://avatars.plurk.com/{user_id}-small{avatar}.gif 
http://avatars.plurk.com/{user_id}-medium{avatar}.gif 
http://avatars.plurk.com/{user_id}-big{avatar}.jpg

If has_profile_image == 0:
http://www.plurk.com/static/default_small.gif 
http://www.plurk.com/static/default_medium.gif 
http://www.plurk.com/static/default_big.gif
  */
class PlurkApiUser
{
public:
    PlurkApiUser( QString user_id,
                  QString user_nick_name,
                  QString user_display_name,
                  bool user_has_profile_image,
                  int user_gender,
                  QString user_avatar)
            : id( user_id),
              nick_name( user_nick_name),
              display_name( user_display_name), 
              has_profile_image( user_has_profile_image), 
              gender( user_gender),
	      avatar( user_avatar) {}
    // The unique user id.
    QString id;
    // The unique nick_name of the user, for example amix.
    QString nick_name;
    // The non-unique display name of the user, for example Amir S. 
    // Only set if it's non empty.
    QString display_name;
    // If 1 then the user has a profile picture, 
    // otherwise the user should use the default.
    bool has_profile_image;
    // Specifies what the latest avatar (profile picture) version is.
    QString avatar;
    // The user's location, a text string, for example Aarhus Denmark.
    QString location;
    // The user's profile language.
    QString default_lang;
    // The user's birthday. ex. "Sat, 19 Mar 1983 00:00:00 GMT"
    QDateTime date_of_birth;
    // The user's full name, like Amir Salihefendic.
    QString full_name;
    // 1 is male, o is female, 2 is not stating/other
    int gender;
    // The profile title of the user.
    QString page_title;
    // User's karma value.
    QString karma;
    // how many friends has the user recruited
    int recruited;
    // Can be not_saying, single, married, divorced, engaged,
    // in_relationship, complicated, widowed, open_relationship
    QString relationship;
};

/*
   A plurk and it's data
   A plurk is encoded as a JSON object. The dates used will be UTC and you are expected to post UTC to the Plurk server as well. You should render the time in user's local time. Typically it will be returned as following:
   {"responses_seen": 0, "qualifier": "thinks", "plurk_id": 90812, "response_count": 0, "limited_to": null, "no_comments": 0, "is_unread": 1, "lang": "en", "content_raw": "test me out", "user_id": 1, "plurk_type": 0, "content": "test me out", "qualifier_translated": "thinks", "posted": "Fri, 05 Jun 2009 23:07:13 GMT", "owner_id": 1, "favorite": false, "favorite_count": 1, "favorers": [3196376], "replurkable": true, "replurked": true, "replurker_id": null, "replurkers": [1], "replurkers_count": 1}

   If &minimal_data=1 is sent as an argument to the request, then some of this data will be removed (this is recommended if you want to optimize bandwidth usage). content_raw and any null attribute will be removed. The above example will look like:
   {"lang": "en", "posted": "Fri, 05 Jun 2009 23:07:13 GMT", "qualifier": "thinks", "plurk_id": 90812, "owner_id": 1, "content": "test me out", "user_id": 1, "is_unread": 1, "no_comments": 0, "plurk_type": 0}
  */
class PlurkApiPost
{
public:
    // The unique Plurk id, used for identification of the plurk.
    QString plurk_id;
    // The English qualifier, can be "says",...
    QString qualifier;
    // Only set if the language is not English, will be the translated qualifier. Can be "siger" if plurk.lang is "da" (Danish).
    QString qualifier_translated;
    /*
       is_unread=0 //Read
       is_unread=1 //Unread
       is_unread=2 //Muted
    */
    int is_unread;
    /*
       plurk_type=0 //Public plurk
       plurk_type=1 //Private plurk
       plurk_type=2 //Public plurk (responded by the logged in user)
       plurk_type=3 //Private plurk (responded by the logged in user)
    */
    int plurk_type;
    // Which timeline does this Plurk belong to.
    QString user_id;
    // Who is the owner/poster of this plurk.
    QString owner_id;
    // The date this plurk was posted.
    QDateTime posted;
    /*
       If set to 1, then responses are disabled for this plurk.
       If set to 2, then only friends can respond to this plurk.
    */
    int no_comments;
    // The formatted content, emoticons and images will be turned into IMG tags etc.
    QString content;
    // The raw content as user entered it, useful when editing plurks or if you want to format the content differently.
    QString content_raw;
    // How many responses does the plurk have.
    int response_count;
    // How many of the responses have the user read. This is automatically updated when fetching responses or marking a plurk as read.
    int responses_seen;
    /*
       If the Plurk is public limited_to is null. If the Plurk is posted to a
       user's friends then limited_to is [0]. If limited_to is [1,2,6,3] then
       it's posted only to these user ids.
    */
    QStringList limited_to;
    // True if current user has liked given plurk.
    bool favorite;
    // Number of users who liked given plurk.
    int favorite_count;
    // List of ids of users who liked given plurk (can be truncated).
    QStringList favorers;
    // True if plurk can be replurked.
    bool replurkable;
    // True if plurk has been replurked by current user.
    bool replurked;
    // ID of a user who has replurked given plurk to current user's timeline.
    QString replurker_id;
    // Number of users who replurked given plurk.
    int replurkers_count;
    // List of ids of users who replurked given plurk (can be truncated).
    QStringList replurkers;
};

#endif
