From gmail.com!coldspacemachine+caf_=dam=rightcore.com Sat Feb 28 16:16:01 USEST 2015 -0500
return-path:  <coldspacemachine+caf_=dam=rightcore.com@gmail.com>
received: from mail-ie0-f172.google.com (mail-ie0-f172.google.com 
	[209.85.223.172])
 by mail.rightcore.com (RCMAILER) with ESMTP id 9A3B62EF03
 for <dam@rightcore.com>; Sat, 28 Feb 2015 16:16:00 -0500 (USEST)
received: by iecar1 with SMTP id ar1so39271144iec.0
 for <dam@rightcore.com>; Sat, 28 Feb 2015 13:15:59 -0800 (PST)
received: by 10.50.0.33 with SMTP id 1csp4643146igb;
 Sat, 28 Feb 2015 13:15:58 -0800 (PST)
received: from mail-lb0-x236.google.com (mail-lb0-x236.google.com. 
	[2a00:1450:4010:c04::236])
 by mx.google.com with ESMTPS id e12si5502945lbq.46.2015.02.28.13.15.57
 for <coldspacemachine@gmail.com>
 (version=TLSv1.2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
 Sat, 28 Feb 2015 13:15:58 -0800 (PST)
received: by mail-lb0-x236.google.com with SMTP id n10so22813751lbv.4
 for <coldspacemachine@gmail.com>; Sat, 28 Feb 2015 13:15:57 -0800 (PST)
received: by 10.112.161.1 with SMTP id xo1csp608493lbb;
 Sat, 28 Feb 2015 13:15:56 -0800 (PST)
received: from mail-ob0-x22b.google.com (mail-ob0-x22b.google.com. 
	[2607:f8b0:4003:c01::22b])
 by mx.google.com with ESMTPS id g185si2337500oia.137.2015.02.28.13.15.54
 for <morano@ieee.org>
 (version=TLSv1.2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
 Sat, 28 Feb 2015 13:15:55 -0800 (PST)
received: by mail-ob0-f171.google.com with SMTP id gq1so24793156obb.2
 for <morano@ieee.org>; Sat, 28 Feb 2015 13:15:54 -0800 (PST)
received: by 10.182.135.232 with HTTP; Sat, 28 Feb 2015 13:15:54 -0800 (PST)
content-length: 215                 
content-lines: 5                   
message-id:
 <CANXuW=OxRG2D2JHk6HTWKULKLfpoPnpRD7HQ2qK4kKxR90RnTQ@mail.gmail.com>
x-original-to: dam@rightcore.com
delivered-to: dam@rightcore.com
delivered-to: coldspacemachine@gmail.com
delivered-to: morano@ieee.org
x-received: by 10.50.32.71 with SMTP id g7mr11872212igi.4.1425158159799;
 Sat, 28 Feb 2015 13:15:59 -0800 (PST)
x-received: by 10.112.172.131 with SMTP id 
	bc3mr18383280lbc.79.1425158158173;
 Sat, 28 Feb 2015 13:15:58 -0800 (PST)
x-received: by 10.152.116.18 with SMTP id 
	js18mr18405431lab.106.1425158157476;
 Sat, 28 Feb 2015 13:15:57 -0800 (PST)
x-received: by 10.60.131.238 with SMTP id 
	op14mr14521455oeb.59.1425158155753;
 Sat, 28 Feb 2015 13:15:55 -0800 (PST)
x-received: by 10.202.213.82 with SMTP id m79mr13544746oig.87.1425158154860;
 Sat, 28 Feb 2015 13:15:54 -0800 (PST)
x-forwarded-to: dam@rightcore.com
x-forwarded-to: coldspacemachine@gmail.com
x-forwarded-for: coldspacemachine@gmail.com dam@rightcore.com
received-spf: pass (google.com: domain of 
	morano+caf_=coldspacemachine=gmail.com@ieee.org designates 
	2a00:1450:4010:c04::236 as permitted sender) 
	client-ip=2a00:1450:4010:c04::236;
received-spf: pass (google.com: domain of dell.morano@gmail.com designates 
	2607:f8b0:4003:c01::22b as permitted sender) 
	client-ip=2607:f8b0:4003:c01::22b;
authentication-results: mx.google.com;
 spf=pass (google.com: domain of 
	morano+caf_=coldspacemachine=gmail.com@ieee.org designates 
	2a00:1450:4010:c04::236 as permitted sender) 
	smtp.mail=morano+caf_=coldspacemachine=gmail.com@ieee.org;
 dkim=pass header.i=@gmail.com;
 dmarc=pass (p=NONE dis=NONE) header.from=gmail.com
x-google-dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=1e100.net; s=20130820;
 h=x-gm-message-state:delivered-to:dkim-signature:mime-version:date
 :message-id:subject:from:to:cc:content-type;
 bh=VHAOqKg/t0Xi86ug0Xw5JX7DmCCAY4tx+EDXcme7t8Y=;
 b=XRl4IBt1r069aIMESOhLhHqiErq+HQbRiK9SrMiMV5u5Ls8dZNjo5hBc2tXgJM/rT6
 8Yuex5cdvbnxMkFdINIsMYLWGCy8b35QqfoenMHlAPC2f8yDl97Ne0VvU/QVv8FjPBCQ
 QdTTlBYIH+xpQGaALmOamtLWp+akaLb3PazOG5LdKsTwVQ4o1a/45I3R9JconlgAIzjJ
 aMlGp1SE7C/tByVE4x57Mo/hhgUm4XMhtfOVSqwDQESxEiPTUzbcDsJdwjzW8viQGsAB
 9ktrqGS8SAissViy/nGGpYzjoLJYFD85lV/DCkBoUKa4NG0DDIYb0DrpXwaKsxGoL3Jk
 DWFw==
x-gm-message-state: 
	ALoCoQlKcGnVOYs98MwprZjBt3+TNDZsG5/cxtc7CZCVDVcGwn3ZmlZVbPwGmGH8jC/LyDPZ3v9t
x-x-forwarded-for: morano@ieee.org coldspacemachine@gmail.com
dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=gmail.com; s=20120113;
 h=mime-version:date:message-id:subject:from:to:cc:content-type;
 bh=VHAOqKg/t0Xi86ug0Xw5JX7DmCCAY4tx+EDXcme7t8Y=;
 b=AKUYHgYRz/AIPpFzmo8XWb9khVTDMW0uJP4dHmlmbWxd7fL708XH4MvByWh3kUBE/1
 MjdhOv2gUdUBFZ6dEEO865QDXfRKzfQEH0UdzRpxgbDieRJRJ4jlp+Nr6CJ/XXt2VzUw
 0XNX1cS8vigs3Y+cfJoNjMaWI7rm9gANDYPX9jbZMPZwEl/IpADhJ9FiV19QAD+k/cOn
 QL6oFHEZRC8+NNlRrwQWtOUTRsogqI32K/81jn9lkKsxnw3Db4zbCXeOcaqoSf5Y8YaL
 wO8+L0iYRYdm8oopzrSpLnowlpXO1aX+ZiPHS5PW2oSFqNW6CBzMuac0yvxn6miId/wB
 IJ2g==
mime-version: 1.0
content-type: text/plain; charset=ISO-8859-1
x-bogosity: Unsure, tests=bogofilter, spamicity=0.520000, version=1.1.5
status:           
From:  "Dell Morano" <dell.morano@gmail.com>,
	"Other Person" <person@rightcore.com> (Person from  the  valley)
To:  prefix"D\\avid \"Morano:ACM" <morano@acm.org>, "David Morano: IEEE"
 <morano@ieee.org>, "David Morano: COMPUTER" <morano@computer.org>, 
	"Philip Dell'Orfano Morano" <pmorano@gmail.com>, "Joe Meszaros"
 <Joe.Meszaros@verizon.net> (This is   	 Joe   Meszaros  	)   
cc:  "Dell Morano" <dell.morano@gmail.com>
Date: Sat, 28 Feb 2015 16:15:54 -0500
Subject: reason...

The only reason I feel a little timid is because my new found
friend...is a Consultant/for Special Products and works in Chestnut
Hill ... she wanted me to have her business card ---I possess her
business card.
mom
