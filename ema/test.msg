From gmail.com!coldspacemachine+caf_=dam=rightcore.com Mon Jul 18 06:00:19 EDT 2016 -0400
return-path:  <coldspacemachine+caf_=dam=rightcore.com@gmail.com>
received: from mail-oi0-f54.google.com (mail-oi0-f54.google.com 
	[209.85.218.54])
 by mail.rightcore.com (RCMAILER) with ESMTP id CD7152EEB3
 for <dam@rightcore.com>; Mon, 18 Jul 2016 06:00:18 -0400 (EDT)
received: by mail-oi0-f54.google.com with SMTP id w18so245127391oiw.3
 for <dam@rightcore.com>; Mon, 18 Jul 2016 03:00:19 -0700 (PDT)
received: by 10.202.79.78 with SMTP id d75csp1116449oib;
 Mon, 18 Jul 2016 03:00:17 -0700 (PDT)
received: from acmsmtp02.acm.org (ACMSMTP02.acm.org. [64.238.147.79])
 by mx.google.com with ESMTPS id y135si15669858qky.58.2016.07.18.03.00.17
 for <coldspacemachine@gmail.com>
 (version=TLS1_2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
 Mon, 18 Jul 2016 03:00:17 -0700 (PDT)
received: from in-001.lax.mailroute.net
 by acmsmtp02.acm.org (ACM Email Forwarding Service) with ESMTP id 
	2201607180600147084
 for <morano@acm.org>; Mon, 18 Jul 2016 06:00:14 -0400
received: from localhost (localhost.localdomain [127.0.0.1])
 by in-001.lax.mailroute.net (Postfix) with ESMTP id 3rtJZQ3HTXzgb86
 for <morano@acm.org>; Mon, 18 Jul 2016 10:00:14 +0000 (UTC)
received: from in-001.lax.mailroute.net ([199.89.1.4])
 by localhost (001.lax.mailroute.net [127.0.0.1]) (mroute_mailscanner, port 
	10024)
 with LMTP id gMY11ie46mq0 for <morano@acm.org>;
 Mon, 18 Jul 2016 10:00:14 +0000 (UTC)
received: from vms173021pub.verizon.net (vms173021pub.verizon.net 
	[206.46.173.21])
 (using TLSv1 with cipher DHE-RSA-AES128-SHA (128/128 bits))
 (No client certificate requested)
 by in-001.lax.mailroute.net (Postfix) with ESMTPS id 3rtJZQ2BHzzgb7B
 for <morano@acm.org>; Mon, 18 Jul 2016 10:00:14 +0000 (UTC)
received: from vz-proxy-l001.mx.aol.com ([64.236.82.153])
 by vms173021.mailsrvcs.net
 (Oracle Communications Messaging Server 7.0.5.32.0 64bit (built Jul 16 
	2014))
 with ESMTPA id <0OAI004R894C8J40@vms173021.mailsrvcs.net> for 
	morano@acm.org;
 Mon, 18 Jul 2016 05:00:13 -0500 (CDT)
received: by 71.174.16.106 with SMTP id 4718ca65; Mon, 18 Jul 2016 10:00:13 
	GMT
content-length: 4695                
message-id: <578CA8AC.10205@verizon.net>
x-original-to: dam@rightcore.com
delivered-to: dam@rightcore.com
delivered-to: coldspacemachine@gmail.com
x-google-dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=1e100.net; s=20130820;
 h=x-original-authentication-results:x-gm-message-state:delivered-to
 :mime-version:message-id:date:from:user-agent:to:cc:subject
 :references:in-reply-to;
 bh=sKhIUB/BlVs3Gcm05JoJ51c46dBrBlRceWM07VewfMQ=;
 b=NQcVmIiM64M1c9cYByEDzy+l4KIRiQ6cVfENMqz64OwCOVAP9kVa9z68mrvZMeNAa5
 cC2XNWAXrWZvBJfnl8cM2KBQvpeqxD5AWUj+ppXP6eivnGf9KMVazpP8HiDn9nc6aQkx
 7wEME3SV+MTIi/IM+rtUF6y1lyssxZAd0emyWCBhoTNQpsHgBpOMUqKub1Ptv8XwAVlA
 LW0F/7elvrdxzXNyU4XptIzSzae1dx8BhEvj6p9c/G0qgtYNYJgmZyMVcDlia8lgH2BK
 c+74iOgRFWaZjs1DW470vjoUgEgFlyfZbdXYQwaUmn2nAMY6AwWjYxHaHPjY7nq4lGR0
 xBKw==
x-original-authentication-results: mx.google.com;       spf=pass 
	(google.com: domain of 
	srs0+5149c9641ef418fe=ti=verizon.net=joe.meszaros@srs.acm.org 
	designates 64.238.147.79 as permitted sender) 
	smtp.mailfrom=SRS0+5149c9641ef418fe=TI=verizon.net=joe.meszaros@srs.acm.org
x-gm-message-state: 
	ALyK8tJOIn9FU2uSUeoamZdSbjg2Vsav7bJAo2c3e5WuOcI2LyxI6t1XEH6ud/mOm55hLHpzpthLYxiYF2tkK1ZgBmKR8Fc=
x-received: by 10.202.77.68 with SMTP id a65mr16217916oib.9.1468836018780;
 Mon, 18 Jul 2016 03:00:18 -0700 (PDT)
x-received: by 10.55.92.198 with SMTP id 
	q189mr41295142qkb.155.1468836017894;
 Mon, 18 Jul 2016 03:00:17 -0700 (PDT)
x-forwarded-to: dam@rightcore.com
x-forwarded-for: coldspacemachine@gmail.com dam@rightcore.com
received-spf: pass (google.com: domain of 
	srs0+5149c9641ef418fe=ti=verizon.net=joe.meszaros@srs.acm.org 
	designates 64.238.147.79 as permitted sender) 
	client-ip=64.238.147.79;
authentication-results: mx.google.com;
 spf=pass (google.com: domain of 
	srs0+5149c9641ef418fe=ti=verizon.net=joe.meszaros@srs.acm.org 
	designates 64.238.147.79 as permitted sender) 
	smtp.mailfrom=SRS0+5149c9641ef418fe=TI=verizon.net=joe.meszaros@srs.acm.org
x-virus-scanned: by MailRoute
x-spam-flag: NO
x-spam-score: 0
x-spam-level:
x-spam-status: No, score=x tagged_above=-9999 required=7 WHITELISTED 
	tests=[]
 autolearn=unavailable
x-envelope-from: <joe.meszaros@verizon.net>
mime-version: 1.0
content-type: multipart/alternative;
 boundary="Boundary_(ID_hCGiYEyfC4PqH/GjQGoTzw)"
x-cmae-score: 0
x-cmae-analysis: v=2.1 cv=MtGvkDue c=1 sm=1 tr=0	
	a=FJ1kTJ0/xm5uTekQe8vMdQ==:117
 a=cAmyUtKerLwA:10 a=r77TgQKjGQsHNAKrUKIA:9	a=zzCJXWfUc6DOPqLda3wA:9
 a=QEXdDO2ut3YA:10 a=pGLkceISAAAA:8	a=-Le3Xl-DfCALNH84tIoA:9
 a=WeJU_xXJPPjPrWVE:21 a=_W_S_7VecoQA:10
user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.6; rv:17.0)
 Gecko/20130328 Thunderbird/17.0.5
in-reply-to: 
	<CANXuW=OYFj1FB5ePLBxhTknQNz-6UZhDxo-2uHihuqCNUk=5zg@mail.gmail.com>
x-bogosity: Ham, tests=bogofilter, spamicity=0.000000, version=1.1.5
status:           
references: 
	<CANXuW=OYFj1FB5ePLBxhTknQNz-6UZhDxo-2uHihuqCNUk=5zg@mail.gmail.com>
From:  Joe Meszaros <joe.meszaros@verizon.net>
To:  Dell Morano <dell.morano@gmail.com>
cc:  David Morano - IEEE <morano@ieee.org>, David Morano:ACM
 <morano@acm.org>, Philip Dell'Orfano Morano <pmorano@gmail.com>
Date: Mon, 18 Jul 2016 06:00:12 -0400
Subject: Re: next 3 days n

This is a multi-part message in MIME format.

--Boundary_(ID_hCGiYEyfC4PqH/GjQGoTzw)
Content-type: text/plain; CHARSET=US-ASCII; format=flowed
Content-transfer-encoding: 7BIT

'
I believe one officer was black and the other two white. I might be 
mistaken, no time to check now.

As I've been saying, all of this is due to the long term lack of 
leadership from the Right, allowing the Left to go on with crazier and 
crazier things. Also, the philosopher Hegel said that with every major 
cultural change (the end of racism) the "pendulum" swings too far the 
other way before settling. So we may be in a necessary "atonement" 
period right now.

On 7/17/16 11:21 PM, Dell Morano wrote:
> Dave-Phil-Joe,
>
> There have been several  protest arrests already in Cleveland 
> -crossing the barrier lines....and the Republican Convention starts 
> tomorrow, Monday.....so if ya can...keep an eye on the Convention for 
> the next three days...
>
> Yeah...Dave, you're right bout the LA shooter - he was Blk...and I 
> still dunno if the dead officers are Blk or white..
>
> Sometimes I wonder if the violent times we are witnessing....is just 
> the beginning of more to come. The people  seem restless and their 
> needs are not met - so they take their anger out on society. fighting 
> for their cause .  I wonder what the Bible has to say about such 
> behavior or has it been already predicted.?..
>
> Do ya think many of the crimes - caused are from the freedom of all 
> the drugs brought into our country and those that take such drugs just 
> don't know or care what they are doing?
>
> Do you guys have any ideas on what is causing the restlessness? Could 
> it be those who have recently been released from jails are having a 
> hard time adjusting to society outside the jail walls?..
> mom


--Boundary_(ID_hCGiYEyfC4PqH/GjQGoTzw)
Content-type: text/html; charset=UTF-8
Content-transfer-encoding: 8BIT

<html>
  <head>
    <meta content="text/html; charset=UTF-8" http-equiv="Content-Type">
  </head>
  <body bgcolor="#FFFFFF" text="#000000">
    '<br>
    I believe one officer was black and the other two white. I might be
    mistaken, no time to check now.<br>
    <br>
    As I've been saying, all of this is due to the long term lack of
    leadership from the Right, allowing the Left to go on with crazier
    and crazier things. Also, the philosopher Hegel said that with every
    major cultural change (the end of racism) the "pendulum" swings too
    far the other way before settling. So we may be in a necessary
    "atonement" period right now.<br>
    <br>
    <div class="moz-cite-prefix">On 7/17/16 11:21 PM, Dell Morano wrote:<br>
    </div>
    <blockquote
cite="mid:CANXuW=OYFj1FB5ePLBxhTknQNz-6UZhDxo-2uHihuqCNUk=5zg@mail.gmail.com"
      type="cite">
      <div dir="ltr">
        <div class="gmail_default" style="color:rgb(0,0,0)">Dave-Phil-Joe,<br>
          <br>
        </div>
        <div class="gmail_default" style="color:rgb(0,0,0)">There have
          been several  protest arrests already in Cleveland -crossing
          the barrier lines....and the Republican Convention starts
          tomorrow, Monday.....so if ya can...keep an eye on the 
          Convention for the next three days...<br>
          <br>
        </div>
        <div class="gmail_default" style="color:rgb(0,0,0)">Yeah...Dave,
          you're right bout the LA shooter - he was <span tabindex="-1"
            id=":1jo.1" style="" class="">Blk</span>...and I still dunno
          if the dead officers are <span tabindex="-1" id=":1jo.2"
            style="" class="">Blk</span> or white..<br>
          <br>
          Sometimes I wonder if the violent times we are
          witnessing....is just the beginning of more to come. The
          people  seem restless and their needs are not met - so they
          take their anger out on society. fighting for their cause .  I
          wonder what the Bible has to say about such behavior or has it
          been already predicted.?..<br>
          <br>
          Do ya think many of the crimes - caused are from the freedom
          of all the drugs brought into our country and those that take
          such drugs just don't know or care what they are doing? <br>
          <br>
          Do you guys have any ideas on what is causing the
          restlessness? Could it be those who have recently been
          released from jails are having a hard time adjusting to
          society outside the jail walls?..<br>
        </div>
        <div class="gmail_default" style="color:rgb(0,0,0)">mom<br>
        </div>
      </div>
    </blockquote>
    <br>
  </body>
</html>

--Boundary_(ID_hCGiYEyfC4PqH/GjQGoTzw)--
