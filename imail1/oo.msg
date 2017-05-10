From gmail.com!coldspacemachine+caf_=dam=rightcore.com Thu Jan 12 01:01:35 EST 2017 -0500
return-path:  <coldspacemachine+caf_=dam=rightcore.com@gmail.com>
received: from mail-io0-f171.google.com (mail-io0-f171.google.com 
	[209.85.223.171])
 by mail.rightcore.com (RCMAILER) with ESMTP id 732E82EF45
 for <dam@rightcore.com>; Thu, 12 Jan 2017 01:01:33 -0500 (EST)
received: by mail-io0-f171.google.com with SMTP id j13so10930052iod.3
 for <dam@rightcore.com>; Wed, 11 Jan 2017 22:01:32 -0800 (PST)
received: by 10.36.57.136 with SMTP id l130csp502223ita;
 Wed, 11 Jan 2017 22:01:29 -0800 (PST)
received: from spmpdv01.ieee.org (smtp1.ieee.org. [54.208.157.121])
 by mx.google.com with ESMTPS id o24si5394435qko.328.2017.01.11.22.01.29
 for <coldspacemachine@gmail.com>
 (version=TLS1_2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
 Wed, 11 Jan 2017 22:01:29 -0800 (PST)
received: from cs-listserv.ieee.org (cs-listserv.ieee.org [54.208.189.96])
 by spmpdv01.ieee.org (8.14.4/8.14.4/Debian-4+deb7u1) with ESMTP id 
	v0C61IBF020391
 for <coldspacemachine@gmail.com>; Thu, 12 Jan 2017 01:01:29 -0500
received: from spmpdv02.ieee.org (spmpdv02.ieee.org [10.144.194.218])
 by cs-listserv.ieee.org (8.14.4/8.14.4/) with ESMTP id v0C60uZK032043
 for <morano@computer.org>; Thu, 12 Jan 2017 01:00:56 -0500
received: from mail-qt0-f198.google.com (mail-qt0-f198.google.com 
	[209.85.216.198])
 by spmpdv02.ieee.org (8.14.4/8.14.4/Debian-4+deb7u1) with ESMTP id 
	v0C60jwa025364
 for <morano@computer.org>; Thu, 12 Jan 2017 01:00:50 -0500
received: by mail-qt0-f198.google.com with SMTP id a29so7584596qtb.6
 for <morano@computer.org>; Wed, 11 Jan 2017 22:00:45 -0800 (PST)
received: from lists.wikimedia.org (lists.wikimedia.org. [208.80.154.75])
 by mx.google.com with ESMTPS id 40si5406919qtm.258.2017.01.11.22.00.45
 for <morano@computer.org>
 (version=TLS1_2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
 Wed, 11 Jan 2017 22:00:45 -0800 (PST)
received: from localhost ([::1]:52382 helo=fermium.wikimedia.org)
 by fermium.wikimedia.org with esmtp (Exim 4.84_2)
 (envelope-from <daily-article-l-bounces@lists.wikimedia.org>)
 id 1cRYQk-0007bl-AE; Thu, 12 Jan 2017 06:00:10 +0000
content-length: 4572                
message-id:
 <mailman.420958.1484200808.30487.daily-article-l@lists.wikimedia.org>
x-original-to: dam@rightcore.com
delivered-to: dam@rightcore.com
delivered-to: coldspacemachine@gmail.com
x-google-dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=1e100.net; s=20161025;
 h=x-gm-message-state:delivered-to:dkim-signature:dkim-signature:date
 :mime-version:to:message-id:from:subject:precedence:list-id
 :list-unsubscribe:list-archive:list-post:list-help:list-subscribe
 :reply-to:content-transfer-encoding:errors-to:sender;
 bh=IrR4zUxEYPFxPqNY4TiDu5tuELv1vADFSRtpGUNOfyg=;
 b=T/aGYwwf7/QOYfIe89yWpxs6FRuOIQgrvRGQMxxx84Gg4T6DMi8AmbqQzF+2JNUPmV
 w+d7yRpyvLAoNjciZsu/HnJS6pr+BrwOqCjvS92cS2zIVVFp2helJI5bxbSoLz2HiNUh
 DyZ6lil7YxirCCUwdBxsdlEyR5CGyt7j6fOPpto/AWE2kXPwXTN9sJ+7ClHikca9dmZx
 2VR38bDqrv8DH3RuqiZcLlrHO5Q+twFKRyw4RsAEDQLp8cSR9jciv4Bmbl3v0eTafIbe
 uE7cn1O5DPCYbaa1vepbAG2fHSxWOlK/nEphVo7zsVRhsmbdNe9Cje/rfx+fZ7rSxCBt
 gqvQ==
x-gm-message-state: 
	AIkVDXJ06il9AKxzaU/dBpsAKsRnQomStjGdYiJeYDIymGv8ceW/tn2y31o0xLpcAyrhT2oA6jRnYfWeI8R8/ioA1FChqO8=
x-received: by 10.107.135.42 with SMTP id 
	j42mr11490864iod.171.1484200890449;
 Wed, 11 Jan 2017 22:01:30 -0800 (PST)
x-received: by 10.55.15.231 with SMTP id 100mr11163071qkp.244.1484200889575;
 Wed, 11 Jan 2017 22:01:29 -0800 (PST)
x-received: by 10.55.22.97 with SMTP id g94mr11361371qkh.287.1484200845409;
 Wed, 11 Jan 2017 22:00:45 -0800 (PST)
x-received: by 10.55.22.97 with SMTP id g94mr11361363qkh.287.1484200845302;
 Wed, 11 Jan 2017 22:00:45 -0800 (PST)
x-forwarded-to: dam@rightcore.com
x-forwarded-for: coldspacemachine@gmail.com dam@rightcore.com
received-spf: neutral (google.com: 54.208.157.121 is neither permitted nor 
	denied by domain of daily-article-l-bounces@lists.wikimedia.org) 
	client-ip=54.208.157.121;
received-spf: neutral (spmpdv01.ieee.org: 208.80.154.75 is neither permitted
 nor denied by domain daily-article-l-bounces@lists.wikimedia.org)
 receiver=spmpdv01.ieee.org; client-ip=208.80.154.75;
 envelope-from=<daily-article-l-bounces@lists.wikimedia.org>; 
	helo=cs-listserv.ieee.org;
 identity=mailfrom
authentication-results: mx.google.com;
 dkim=pass header.i=@lists.wikimedia.org;
 dkim=neutral (body hash did not verify) header.i=@gmail.com;
 spf=neutral (google.com: 54.208.157.121 is neither permitted nor denied by 
	domain of daily-article-l-bounces@lists.wikimedia.org) 
	smtp.mailfrom=daily-article-l-bounces@lists.wikimedia.org;
 dmarc=pass (p=NONE sp=NONE dis=NONE) header.from=lists.wikimedia.org
dkim-signature: v=1; a=rsa-sha256; q=dns/txt; c=relaxed/relaxed; 
	d=lists.wikimedia.org; s=wikimedia;
 h=Sender:Content-Transfer-Encoding:Content-Type:Reply-To:List-Subscribe:List-Help:List-Post:List-Archive:List-Unsubscribe:List-Id:Subject:From:Message-ID:To:MIME-Version:Date;
	bh=IrR4zUxEYPFxPqNY4TiDu5tuELv1vADFSRtpGUNOfyg=;
 b=jqt6EMFbNcIyobr+BKrvkR89wTKgDrqRmm/TGMfV0JigxZjxvXclfxjcuoVo80awIUGojgORnw6RE+eqTMi6hxmZQUFhx1rRiuSxcGVL3OJhfwxbE5TRqyUOBL6e4MhZYgVgnuvHzOnsoVCrxKfuYK11d0Bu8Wr5zADJzO2SSFU=;
dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=gmail.com; 
	s=20161025;
 h=message-id:date:mime-version:content-transfer-encoding:from:subject
 :to; bh=AVlhOht3GrS+/6JL70MKSijHVyjXtRzIeKG2cW4MnWQ=;
 b=BjMyLhb7U82Nr9wngo00/PeA1xQD0rU6Ij94vr6LBGUl9sFQebU+VMp0OsaaN7r/SW
 a0GDRxzH952nBZpNlRnPr+N/vPu9o4uqKMZXrwFABFMyTSxMNx6qut+YwtDE45rFooGW
 r5uuopEHKX7u+E9EjiybhjRG2SGq21nXZl3fQUNYgK+5FF7hrMCEYZXGdxhDbH/8OYjX
 xGNiDWwdqr6qHZVAvvFaGfsfPhTOZX0U6BPWi4vPs2F8iyz9KdTZliMGdN9Nu138PcEb
 5DJwb52RLU1c0gG/BPAAw8pXc67lb5PM5+BNJno4GSd6K0MZYGB3xEcW13RkS/lmc74I
 OXXA==
mime-version: 1.0
x-beenthere: daily-article-l@lists.wikimedia.org
x-mailman-version: 2.1.18
precedence: list
list-id: English Wikipedia Article of the Day
 <daily-article-l.lists.wikimedia.org>
list-unsubscribe: 
	<https://lists.wikimedia.org/mailman/options/daily-article-l>,
 <mailto:daily-article-l-request@lists.wikimedia.org?subject=unsubscribe>
list-archive: <https://lists.wikimedia.org/pipermail/daily-article-l/>
list-post: <mailto:daily-article-l@lists.wikimedia.org>
list-help: <mailto:daily-article-l-request@lists.wikimedia.org?subject=help>
list-subscribe: 
	<https://lists.wikimedia.org/mailman/listinfo/daily-article-l>,
 <mailto:daily-article-l-request@lists.wikimedia.org?subject=subscribe>
reply-to: dal-feedback@wikimedia.org
content-type: text/plain; charset="utf-8"
content-transfer-encoding: base64
errors-to: daily-article-l-bounces@lists.wikimedia.org
sender: "daily-article-l" <daily-article-l-bounces@lists.wikimedia.org>
x-gm-spam: 0
x-gm-phishy: 0
x-bayes-prob: 0.9999 (Score 5, tokens from: morano@computer.org, CS_DEFAULT,
	default, @@RPTN)
x-spam-score: undef - spam scanning disabled
x-canit-geo: ip=208.80.154.75; country=US; region=California; city=San 
	Francisco; latitude=37.7957; longitude=-122.4209; 
	http://maps.google.com/maps?q=37.7957,-122.4209&z=6
x-canitpro-stream: Outbound (inherits from default)
x-canit-stats-id: Bayes signature not available
x-scanned-by: CanIt (https:// cuce . ieee . org) on 10.144.192.218
x-scanned-by: CanIt (https:// cuce . ieee . org) on 10.144.194.218
x-bogosity: Unsure, tests=bogofilter, spamicity=0.486918, version=1.1.5
status:           
From:  English Wikipedia Article of the Day
 <daily-article-l@lists.wikimedia.org>
To:  daily-article-l@lists.wikimedia.org
Date: Wed, 11 Jan 2017 22:00:03 -0800 (PST)
Subject: [Daily article] =?utf-8?q?January_12=3A_The_Monster_=28novella=29?=

VGhlIE1vbnN0ZXIgaXMgYW4gMTg5OCBub3ZlbGxhIGJ5IEFtZXJpY2FuIGF1dGhvciBTdGVwaGVu
IENyYW5lCigxODcx4oCTMTkwMCksIGEgc3R1ZHkgb2YgcHJlanVkaWNlLCBmZWFyIGFuZCBpc29s
YXRpb24gaW4gYSBzbWFsbCB0b3duLgpJbiB0aGUgZmljdGlvbmFsIFdoaWxvbXZpbGxlLCBOZXcg
WW9yaywgYW4gQWZyaWNhbi1BbWVyaWNhbiBjb2FjaG1hbgpuYW1lZCBIZW5yeSBKb2huc29uLCB3
aG8gaXMgZW1wbG95ZWQgYnkgdGhlIHRvd24ncyBwaHlzaWNpYW4sIERyLgpUcmVzY290dCwgYmVj
b21lcyBob3JyaWJseSBkaXNmaWd1cmVkIGFmdGVyIGhlIHNhdmVzIFRyZXNjb3R0J3Mgc29uIGZy
b20KYSBmaXJlLiBXaGVuIEhlbnJ5IGlzIGJyYW5kZWQgYSAibW9uc3RlciIgYnkgdGhlIHRvd24n
cyByZXNpZGVudHMsClRyZXNjb3R0IHZvd3MgdG8gc2hlbHRlciBhbmQgY2FyZSBmb3IgaGltLCBy
ZXN1bHRpbmcgaW4gaGlzIGZhbWlseSdzCmV4Y2x1c2lvbiBmcm9tIHRoZSBjb21tdW5pdHkuIFdo
aWxvbXZpbGxlLCB3aGljaCBpcyB1c2VkIGluIDE0IG90aGVyCkNyYW5lIHN0b3JpZXMsIHdhcyBi
YXNlZCBvbiBQb3J0IEplcnZpcywgTmV3IFlvcmssIHdoZXJlIGhlIHNwZW50IHBhcnQKb2YgaGlz
IHlvdXRoLiBIZSBwcm9iYWJseSB0b29rIGluc3BpcmF0aW9uIGZyb20gc2V2ZXJhbCBsb2NhbCBt
ZW4gd2hvCndlcmUgc2ltaWxhcmx5IGRpc2ZpZ3VyZWQuIE1vZGVybiBjcml0aWNzIGhhdmUgY29u
bmVjdGVkIHRoZW1lcyBvZgpyYWNpYWwgZGl2aXNpb24gaW4gdGhlIHN0b3J5IHRvIHRoZSAxODky
IGx5bmNoaW5nIGluIFBvcnQgSmVydmlzIG9mIGFuCkFmcmljYW4tQW1lcmljYW4gbWFuIG5hbWVk
IFJvYmVydCBMZXdpcy4gVGhlIG5vdmVsbGEgd2FzIGluY2x1ZGVkIGluIFRoZQpNb25zdGVyIGFu
ZCBPdGhlciBTdG9yaWVz4oCUdGhlIGxhc3QgY29sbGVjdGlvbiBvZiBDcmFuZSdzIHdvcmsgdG8g
YmUKcHVibGlzaGVkIGR1cmluZyBoaXMgbGlmZXRpbWUuIEJvdGggdGhlIG5vdmVsbGEgYW5kIHRo
ZSBjb2xsZWN0aW9uCnJlY2VpdmVkIG1peGVkIHJldmlld3MgZnJvbSBjcml0aWNzLCBidXQgVGhl
IE1vbnN0ZXIgaXMgbm93IGNvbnNpZGVyZWQKb25lIG9mIENyYW5lJ3MgYmVzdCB3b3Jrcy4KClJl
YWQgbW9yZTogPGh0dHBzOi8vZW4ud2lraXBlZGlhLm9yZy93aWtpL1RoZV9Nb25zdGVyXyhub3Zl
bGxhKT4KCl9fX19fX19fX19fX19fX19fX19fX19fX19fX19fX18KVG9kYXkncyBzZWxlY3RlZCBh
bm5pdmVyc2FyaWVzOgoKMTU1NDoKCkJheWlubmF1bmcsIHdobyBsYXRlciBhc3NlbWJsZWQgdGhl
IGxhcmdlc3QgZW1waXJlIGluIHRoZQpoaXN0b3J5IG9mIFNvdXRoZWFzdCBBc2lhLCB3YXMgY3Jv
d25lZCBraW5nIG9mIHRoZSBCdXJtZXNlIFRhdW5nb28KRHluYXN0eS4KPGh0dHBzOi8vZW4ud2lr
aXBlZGlhLm9yZy93aWtpL0JheWlubmF1bmc+CgoxNzc3OgoKTWlzc2lvbiBTYW50YSBDbGFyYSBk
ZSBBc8OtcywgYSBTcGFuaXNoIG1pc3Npb24gdGhhdCBmb3JtZWQgdGhlCmJhc2lzIG9mIGJvdGgg
dGhlIGNpdHkgb2YgU2FudGEgQ2xhcmEsIENhbGlmb3JuaWEgYW5kIFNhbnRhIENsYXJhClVuaXZl
cnNpdHksIHdhcyBlc3RhYmxpc2hlZC4KPGh0dHBzOi8vZW4ud2lraXBlZGlhLm9yZy93aWtpL01p
c3Npb25fU2FudGFfQ2xhcmFfZGVfQXMlQzMlQURzPgoKMTkxMToKClRoZSBVbml2ZXJzaXR5IG9m
IHRoZSBQaGlsaXBwaW5lcyBDb2xsZWdlIG9mIExhdyB3YXMgZm91bmRlZDsKbWFueSBsZWFkaW5n
IEZpbGlwaW5vIHBvbGl0aWNhbCBmaWd1cmVzIGhhdmUgc2luY2UgZ3JhZHVhdGVkIGZyb20gaXQu
CjxodHRwczovL2VuLndpa2lwZWRpYS5vcmcvd2lraS9Vbml2ZXJzaXR5X29mX3RoZV9QaGlsaXBw
aW5lc19Db2xsZWdlX29mX0xhdz4KCjE5Njc6CgpTZXZlbnR5LXRocmVlLXllYXItb2xkIHBzeWNo
b2xvZ3kgcHJvZmVzc29yIEphbWVzIEJlZGZvcmQKYmVjYW1lIHRoZSBmaXJzdCBwZXJzb24gdG8g
YmUgY3J5b25pY2FsbHkgZnJvemVuIHdpdGggaW50ZW50IG9mIGZ1dHVyZQpyZXN1c2NpdGF0aW9u
Lgo8aHR0cHM6Ly9lbi53aWtpcGVkaWEub3JnL3dpa2kvQ3J5b25pY3M+CgoyMDA3OgoKQ29tZXQg
TWNOYXVnaHQgcmVhY2hlZCBwZXJpaGVsaW9uIGFuZCBiZWNhbWUgdGhlIGJyaWdodGVzdApjb21l
dCBpbiBvdmVyIDQwIHllYXJzIHdpdGggYW4gYXBwYXJlbnQgbWFnbml0dWRlIG9mIOKIkjUuNS4K
PGh0dHBzOi8vZW4ud2lraXBlZGlhLm9yZy93aWtpL0MvMjAwNl9QMT4KCl9fX19fX19fX19fX19f
X19fX19fX19fX19fX19fCldpa3Rpb25hcnkncyB3b3JkIG9mIHRoZSBkYXk6CgpmaWxpYnVzdGVy
OgoxLiBBIG1lcmNlbmFyeSBzb2xkaWVyOyBhIGZyZWVib290ZXI7IHNwZWNpZmljYWxseSwgYSBt
ZXJjZW5hcnkgd2hvCnRyYXZlbGxlZCBpbGxlZ2FsbHkgaW4gYW4gb3JnYW5pemVkIGdyb3VwIGZy
b20gdGhlIFVuaXRlZCBTdGF0ZXMgdG8gYQpjb3VudHJ5IGluIENlbnRyYWwgQW1lcmljYSBvciB0
aGUgU3BhbmlzaCBXZXN0IEluZGllcyBpbiB0aGUgbWlkLTE5dGgKY2VudHVyeSBzZWVraW5nIGVj
b25vbWljIGFuZCBwb2xpdGljYWwgYmVuZWZpdHMgdGhyb3VnaCBhcm1lZCBmb3JjZS4KMi4gKHBv
bGl0aWNzLCBVUykgQSB0YWN0aWMgKHN1Y2ggYXMgZ2l2aW5nIGxvbmcsIG9mdGVuIGlycmVsZXZh
bnQgc3BlZWNoZXMpCmVtcGxveWVkIHRvIGRlbGF5IHRoZSBwcm9jZWVkaW5ncyBvZiwgb3IgdGhl
IG1ha2luZyBvZiBhIGRlY2lzaW9uIGJ5LCBhCmxlZ2lzbGF0aXZlIGJvZHksIHBhcnRpY3VsYXJs
eSB0aGUgVW5pdGVkIFN0YXRlcyBTZW5hdGUuCjMuIChwb2xpdGljcywgVVMpIEEgbWVtYmVyIG9m
IGEgbGVnaXNsYXRpdmUgYm9keSBjYXVzaW5nIHN1Y2ggYW4Kb2JzdHJ1Y3Rpb247IGEgZmlsaWJ1
c3RlcmVyLgo8aHR0cHM6Ly9lbi53aWt0aW9uYXJ5Lm9yZy93aWtpL2ZpbGlidXN0ZXI+CgpfX19f
X19fX19fX19fX19fX19fX19fX19fX18KV2lraXF1b3RlIHF1b3RlIG9mIHRoZSBkYXk6CgrCoCAg
ICBUaGUgcGVvcGxlIG5ldmVyIGdpdmUgdXAgdGhlaXIgbGliZXJ0aWVzIGJ1dCB1bmRlciBzb21l
IGRlbHVzaW9uLgrCoAogIC0tRWRtdW5kIEJ1cmtlCjxodHRwczovL2VuLndpa2lxdW90ZS5vcmcv
d2lraS9FZG11bmRfQnVya2U+CgpfX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19f
X19fX19fX19fXwpXaWtpcGVkaWEgRGFpbHkgQXJ0aWNsZSBtYWlsaW5nIGxpc3QuClRvIHVuc3Vi
c2NyaWJlLCB2aXNpdDoKaHR0cHM6Ly9saXN0cy53aWtpbWVkaWEub3JnL21haWxtYW4vbGlzdGlu
Zm8vZGFpbHktYXJ0aWNsZS1sClF1ZXN0aW9ucyBvciBjb21tZW50cz8gQ29udGFjdCBkYWwtZmVl
ZGJhY2tAd2lraW1lZGlhLm9yZw==
