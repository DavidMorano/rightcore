Delivered-To: coldspacemachine@gmail.com
Received: by 10.202.178.69 with SMTP id b66csp1320772oif;
        Sun, 8 May 2016 18:06:57 -0700 (PDT)
X-Received: by 10.55.74.141 with SMTP id x135mr11019778qka.20.1462756017822;
        Sun, 08 May 2016 18:06:57 -0700 (PDT)
Return-Path: <SRS0+8b2ecc2a3576621d=RC=gmail.com=eve.s.silver@srs.acm.org>
Received: from acmsmtp02.acm.org (ACMSMTP02.acm.org. [64.238.147.79])
        by mx.google.com with ESMTPS id d88si17104693qgf.38.2016.05.08.18.06.57
        for <coldspacemachine@gmail.com>
        (version=TLS1_2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
        Sun, 08 May 2016 18:06:57 -0700 (PDT)
Received-SPF: pass (google.com: domain of SRS0+8b2ecc2a3576621d=RC=gmail.com=eve.s.silver@srs.acm.org designates 64.238.147.79 as permitted sender) client-ip=64.238.147.79;
Authentication-Results: mx.google.com;
       dkim=pass header.i=@gmail.com;
       spf=pass (google.com: domain of SRS0+8b2ecc2a3576621d=RC=gmail.com=eve.s.silver@srs.acm.org designates 64.238.147.79 as permitted sender) smtp.mailfrom=SRS0+8b2ecc2a3576621d=RC=gmail.com=eve.s.silver@srs.acm.org
Return-Path: <eve.s.silver@gmail.com>
Received: from in-003.lax.mailroute.net
        by acmsmtp02.acm.org (ACM Email Forwarding Service) with ESMTP id 2201605082106544950
        for <morano@acm.org>; Sun, 08 May 2016 21:06:54 -0400
Received: from localhost (localhost.localdomain [127.0.0.1])
	by in-003.lax.mailroute.net (Postfix) with ESMTP id 3r343M3zTzz1vnqW
	for <morano@acm.org>; Mon,  9 May 2016 01:06:55 +0000 (UTC)
X-Virus-Scanned: by MailRoute
X-Spam-Flag: NO
X-Spam-Score: 0
X-Spam-Level:
X-Spam-Status: No, score=x tagged_above=-9999 required=7 WHITELISTED tests=[]
	autolearn=unavailable
Authentication-Results: 003.lax.mailroute.net (mroute_mailscanner);
	dkim=pass (2048-bit key) header.d=gmail.com
Received: from in-003.lax.mailroute.net ([199.89.1.6])
	by localhost (003.lax.mailroute.net [127.0.0.1]) (mroute_mailscanner, port 10024)
	with LMTP id c-eIo-aCN87i for <morano@acm.org>;
	Mon,  9 May 2016 01:06:55 +0000 (UTC)
X-Envelope-From: <eve.s.silver@gmail.com>
Received: from mail-ig0-f176.google.com (mail-ig0-f176.google.com [209.85.213.176])
	(using TLSv1.2 with cipher ECDHE-RSA-AES128-GCM-SHA256 (128/128 bits))
	(No client certificate requested)
	by in-003.lax.mailroute.net (Postfix) with ESMTPS id 3r343M2Xblz1vnkN
	for <morano@acm.org>; Mon,  9 May 2016 01:06:55 +0000 (UTC)
Received: by mail-ig0-f176.google.com with SMTP id s8so80711017ign.0
        for <morano@acm.org>; Sun, 08 May 2016 18:06:55 -0700 (PDT)
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
        d=gmail.com; s=20120113;
        h=mime-version:sender:from:date:message-id:subject:to;
        bh=cmV2muDm6dssJPL0sJ+OGAmaz8aTOJf3lOcOa91C/ro=;
        b=QkOBUoizWNaUZGbegYwnSBLTkG/ZvkxyHAvZpx2duyX5eSRKCY72EtEWTRh94anh6f
         cvxZMDKzrdfcg6Ued+q+UfmiO4XxeN2zcfrwWA2TQHkFUEGjwOcTBscJTDQEpUhP7EHN
         VgFtKnCUOwMhPSrId2se934SUimhgsVVyLtj26YYUYK7q897qZgSvyDd8CxVJCPm7xno
         sWI/ubsZKPnrjulMfWp3xWIx0m+hCXAxGLwjy65Q0haef1fP/4HcPRuHdvkkDpHSmTW2
         reCO1laSQgY+MiWeoCJ4HCfMIDq+Qg3YKTSGPn7exZ4LdAcK8U68TOlxL0r5Lj3pAt17
         sR9Q==
X-Google-DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
        d=1e100.net; s=20130820;
        h=x-gm-message-state:mime-version:sender:from:date:message-id:subject
         :to;
        bh=cmV2muDm6dssJPL0sJ+OGAmaz8aTOJf3lOcOa91C/ro=;
        b=DzMFSILP3f+0hG5GlDWDqK3LybVZcbqShap9vm7drwZ6Y6P03QmcnaWvrla3+7PPRw
         AFDKEV7KBMqsxMoI/vEnR/t50YLNHlgyE63WVoBZMT8uQT9YLkkHsmN1BrstO9lrXQap
         K0giOX1cTE9iQAmLNUaZd8klk44tGfLH3/Nfcg47R29v7GStk3IsdYXaoSwSU06vBSyi
         Bxj8/pVvGaTwoAGybIsjwnw5O4+jChSQ0kVwXYfreE4QCo/DiOyzAIK7ND27df/OpTQC
         zmX1HlFarFdDT2XHDFtmHXSC3JDM/w/w4zwJ2q3789BZcDgpNu21Ox6qiOoeHNeXqUZo
         PypQ==
X-Gm-Message-State: AOPr4FVP3hflA9+LX8oYL5N6QAxjRaatMrH4+22QrqxR/41ugD1W1nnqhElihKX66W01w3JqpD34kUvc3UsVOg==
X-Received: by 10.50.61.234 with SMTP id t10mr8735233igr.10.1462756014721;
 Sun, 08 May 2016 18:06:54 -0700 (PDT)
MIME-Version: 1.0
Sender: eve.s.silver@gmail.com
Received: by 10.107.141.130 with HTTP; Sun, 8 May 2016 18:06:15 -0700 (PDT)
From: Eve Silver <eve@evesilver.com>
Date: Sun, 8 May 2016 21:06:15 -0400
X-Google-Sender-Auth: l5mDoEKAUAbIvgocjj_taSSsYGU
Message-ID: <CACZC8kODjvT18B0vbqqH_5NdYZZQWDduh25WT9+N7HvL-tAJjg@mail.gmail.com>
Subject: Any news about the MP4? eom
To: David Anthony Morano <morano@acm.org>, David Anthony Morano <morano@computer.org>, 
	David Anthony Morano <morano@ieee.org>
Content-Type: multipart/alternative; boundary=047d7bdc07e2a692f905325e6c25

--047d7bdc07e2a692f905325e6c25
Content-Type: text/plain; charset=UTF-8

Eve

--047d7bdc07e2a692f905325e6c25
Content-Type: text/html; charset=UTF-8

<div dir="ltr"><br clear="all"><div><div class="gmail_signature"><br>Eve</div></div>
</div>

--047d7bdc07e2a692f905325e6c25--
