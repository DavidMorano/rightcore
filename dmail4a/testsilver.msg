From gmail.com!coldspacemachine+caf_=dam=rightcore.com Tue Jul 08 12:58:53 EDT 2014 -0400
return-path:  <coldspacemachine+caf_=dam=rightcore.com@gmail.com>
received: from mail-ig0-f181.google.com (mail-ig0-f181.google.com 
	[209.85.213.181])
 by mail.rightcore.com (RCMAILER) with ESMTP id 1E3432EEF2
 for <dam@rightcore.com>; Tue,  8 Jul 2014 12:58:52 -0400 (EDT)
received: by mail-ig0-f181.google.com with SMTP id h15so898748igd.8
 for <dam@rightcore.com>; Tue, 08 Jul 2014 09:58:52 -0700 (PDT)
received: by 10.50.225.134 with SMTP id rk6csp299383igc;
 Tue, 8 Jul 2014 09:58:51 -0700 (PDT)
received: from cs-ems3.ieee.org (cs-ems3.ieee.org. [140.98.193.209])
 by mx.google.com with ESMTPS id n49si33825578qge.3.2014.07.08.09.58.51
 for <coldspacemachine@gmail.com>
 (version=TLSv1 cipher=RC4-SHA bits=128/128);
 Tue, 08 Jul 2014 09:58:51 -0700 (PDT)
received: from hormel10.ieee.org (hormel10.ieee.org [140.98.193.233])
 by cs-ems3.ieee.org (8.13.8/8.13.8/) with ESMTP id s68GwoEQ005261
 for <morano@computer.org>; Tue, 8 Jul 2014 12:58:50 -0400
received: from spmpdv02.ieee.org (smtp2.ieee.org [54.208.149.44])
 by hormel10.ieee.org (8.14.3/8.14.3/Debian-5+lenny1) with ESMTP id 
	s68JuqOr001983
 for <morano@computer.org>; Tue, 8 Jul 2014 15:56:53 -0400
received: from mail-we0-f169.google.com (mail-we0-f169.google.com 
	[74.125.82.169])
 by spmpdv02.ieee.org (8.14.4/8.14.4/Debian-4) with ESMTP id s68Gw3xg015900
 for <morano@computer.org>; Tue, 8 Jul 2014 12:58:03 -0400
received: by mail-we0-f169.google.com with SMTP id t60so6304433wes.14
 for <morano@computer.org>; Tue, 08 Jul 2014 09:58:38 -0700 (PDT)
received: from mail-wg0-x229.google.com (mail-wg0-x229.google.com 
	[2a00:1450:400c:c00::229])
 by mx.google.com with ESMTPS id dq8si3765408wid.17.2014.07.08.09.58.38
 for <morano@computer.org>
 (version=TLSv1 cipher=ECDHE-RSA-RC4-SHA bits=128/128);
 Tue, 08 Jul 2014 09:58:38 -0700 (PDT)
received: by mail-wg0-f41.google.com with SMTP id k14so3571753wgh.24
 for <morano@computer.org>; Tue, 08 Jul 2014 09:58:38 -0700 (PDT)
received: by 10.194.249.200 with HTTP; Tue, 8 Jul 2014 09:58:37 -0700 (PDT)
content-length: 819                 
message-id: <CACZC8kNB+rryPgGLS3JpmAd1=fkYod77BKoUffNd8shu7Ox8YQ@mail.gmail.com>
x-original-to: dam@rightcore.com
delivered-to: dam@rightcore.com
delivered-to: coldspacemachine@gmail.com
x-google-dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=1e100.net; s=20130820;
 h=delivered-to:x-gm-message-state:dkim-signature:mime-version:sender
 :date:message-id:subject:from:to:content-type;
 bh=TdV774m2EM0QioNZclyB7VkhlO7a+PlgmStX5omjTT0=;
 b=Zb2GlCjq3TNfsNIepyjOJK4dGFrDJ8LUw00uNj2e8MzvXv5ICKSe51Mi04E9fp9409
 GGCFVpU2e3UY+w1btHqCnpedZZyN05Qo3to016FmPG2LsSvd7EeMZvPi1z22qVpOqbU+
 53g6K5AxXh62mxi8P91zBkslILLg4l7S1I2HcRySGkhI0o+kre57Qr2u8eGOi6Zw28Q7
 Srieu2xYV3hfNoz2eSJNP1/BpHwfAXTre5kGzAAD7idRjEUMO+u4mbtBuf9ksNql0neE
 6LF8rK5XgkHwW8eRTVdg1QYy2cdckH9l9Nj6OXi2wttxVLRQ3jo3I7uZYn28dTZxnISY
 uqGg==
x-received: by 10.42.106.15 with SMTP id x15mr11088501ico.67.1404838732689;
 Tue, 08 Jul 2014 09:58:52 -0700 (PDT)
x-received: by 10.224.115.3 with SMTP id g3mr62000695qaq.9.1404838731638;
 Tue, 08 Jul 2014 09:58:51 -0700 (PDT)
x-received: by 10.194.121.6 with SMTP id lg6mr5264711wjb.116.1404838718405;
 Tue, 08 Jul 2014 09:58:38 -0700 (PDT)
x-received: by 10.194.121.6 with SMTP id lg6mr5264707wjb.116.1404838718337;
 Tue, 08 Jul 2014 09:58:38 -0700 (PDT)
x-received: by 10.194.6.134 with SMTP id b6mr42565110wja.64.1404838718140;
 Tue, 08 Jul 2014 09:58:38 -0700 (PDT)
x-forwarded-to: dam@rightcore.com
x-forwarded-for: coldspacemachine@gmail.com dam@rightcore.com
received-spf: softfail (google.com: domain of transitioning 
	eve.s.silver@gmail.com does not designate 140.98.193.209 as 
	permitted sender) client-ip=140.98.193.209;
received-spf: pass (google.com: domain of eve.s.silver@gmail.com designates 
	2a00:1450:400c:c00::229 as permitted sender) 
	client-ip=2a00:1450:400c:c00::229;
authentication-results: mx.google.com;
 spf=softfail (google.com: domain of transitioning eve.s.silver@gmail.com 
	does not designate 140.98.193.209 as permitted sender) 
	smtp.mail=eve.s.silver@gmail.com;
 dkim=pass header.i=@gmail.com
x-gm-message-state: 
	ALoCoQkRAqpsuNCMDUjpHxyGB5WmHwkyxUy4NIt/wG+6auoeuuaSVITNzD61J7vH4Ih9VKr7+MrAKH9YAwlSI+WNO44zvaGeUKlJtr/Nm9A4+nn2L1R+5/EZkAaUnqrnrwAfN9zOnPkD
dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=gmail.com; s=20120113;
 h=mime-version:sender:date:message-id:subject:from:to:content-type;
 bh=TdV774m2EM0QioNZclyB7VkhlO7a+PlgmStX5omjTT0=;
 b=ufXLm82f0WSdp6WHCs99dhdHyx4anZ908JBJNeSO5vBduClDDdpOPsJiXYmX9Qy6VA
 Sd54IEKakfvrjaNkDuZmVk3vbd1jRZaOFCpLOO+Pp6a7yfFqeLzFasjB5r6BgJgy6zEs
 SCW+z7+Ychy3HQC9jJwZej/GwUQPSfrICJLpPEhhibxzGdc/5is4wePon6plNDhvIY71
 4C/e7M19WQwNfa4JtCo7bC63BV1H1jqTAEsAthbUMDWuB8lWxl/nPlZFKi7/rkWfM7JM
 2k0dZQ5e09iDJU4SKP7rPvNHo1xWHrO2MCkqbair8fFhad20U5IybuHIeG8aWGVoTZsF
 msEw==
mime-version: 1.0
sender: eve.s.silver@gmail.com
x-google-sender-auth: 4fifyOHAA6V5PklX6wH8e3aJMno
content-type: multipart/alternative; boundary=047d7b450116c2f5f804fdb18095
x-spam-score: -1.00 () [Hold at 7.00] 
	FREEMAIL_FROM,HTML_MESSAGE,SPF(pass:-1)
x-canit-geo: ip=74.125.82.169; country=US; region=California; city=Mountain 
	View; latitude=37.3845; longitude=-122.0881; 
	http://maps.google.com/maps?q=37.3845,-122.0881&z=6
x-canitpro-stream: 80_DEFAULT (inherits from default)
x-canit-stats-id: Bayes signature not available
x-scanned-by: IEEE Spam Scanner (https://uce.ieee.org/) on 140.98.193.233
x-scanned-by: CanIt [cuce . ieee . org] on 10.144.194.218
x-bayes-prob: 0.0001 (Score 0, tokens from: @@RPTN)
status: 
From:  Eve Silver <eve@evesilver.com>
To:  RTTC <morano@computer.org>
Date: Tue, 8 Jul 2014 12:58:37 -0400
Subject: Urgent doctor appt @ 4pm

--047d7b450116c2f5f804fdb18095
Content-Type: text/plain; charset=UTF-8

BAD UTI.

I pray it is under control by tomorrow. Pretty miserable right now ...

I'll let you know what the doctor says. But we might have to cancel our
trip. What do you think?

So sad.

Eve



-- 
Sent from Gmail Mobile

--047d7b450116c2f5f804fdb18095
Content-Type: text/html; charset=UTF-8
Content-Transfer-Encoding: quoted-printable

BAD UTI.=C2=A0<div><br></div><div>I pray it is under control by tomorrow. P=
retty miserable right now ...</div><div><br></div><div>I&#39;ll let you kno=
w what the doctor says. But we might have to cancel our trip. What do you t=
hink?</div>
<div><br></div><div>So sad.=C2=A0</div><div><br></div><div>Eve<br><div><br>=
</div></div><br><br>-- <br>Sent from Gmail Mobile<br>

--047d7b450116c2f5f804fdb18095--
