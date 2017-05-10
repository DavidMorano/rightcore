From gmail.com!coldspacemachine+caf_=dam=rightcore.com Wed Oct 01 01:01:51 USEDT 2014 -0400
return-path:  <coldspacemachine+caf_=dam=rightcore.com@gmail.com>
received: from mail-ig0-f175.google.com (mail-ig0-f175.google.com 
	[209.85.213.175])
 by mail.rightcore.com (RCMAILER) with ESMTP id B71512EEF2
 for <dam@rightcore.com>; Wed,  1 Oct 2014 01:01:50 -0400 (USEDT)
received: by mail-ig0-f175.google.com with SMTP id uq10so140372igb.8
 for <dam@rightcore.com>; Tue, 30 Sep 2014 22:01:51 -0700 (PDT)
received: by 10.51.16.99 with SMTP id fv3csp364990igd;
 Tue, 30 Sep 2014 22:01:50 -0700 (PDT)
received: from cs-listserv.ieee.org (cs-listserv.ieee.org. [54.208.189.96])
 by mx.google.com with ESMTPS id a3si21697020qac.16.2014.09.30.22.01.49
 for <coldspacemachine@gmail.com>
 (version=TLSv1 cipher=RC4-SHA bits=128/128);
 Tue, 30 Sep 2014 22:01:49 -0700 (PDT)
received: from spmpdv02.ieee.org (spmpdv02.ieee.org [10.144.194.218])
 by cs-listserv.ieee.org (8.14.4/8.14.4) with ESMTP id s9151nFA007716
 for <morano@computer.org>; Wed, 1 Oct 2014 01:01:49 -0400
received: from mail-qa0-f51.google.com (mail-qa0-f51.google.com 
	[209.85.216.51])
 by spmpdv02.ieee.org (8.14.4/8.14.4/Debian-4) with ESMTP id s9151l9m006782
 for <morano@computer.org>; Wed, 1 Oct 2014 01:01:47 -0400
received: by mail-qa0-f51.google.com with SMTP id k15so32662qaq.38
 for <morano@computer.org>; Tue, 30 Sep 2014 22:01:47 -0700 (PDT)
received: from lists.wikimedia.org (lists.wikimedia.org. [208.80.154.4])
 by mx.google.com with ESMTP id o66si6089566qge.85.2014.09.30.22.01.47
 for <morano@computer.org>;
 Tue, 30 Sep 2014 22:01:47 -0700 (PDT)
received: from localhost ([::1]:52086 helo=sodium.wikimedia.org)
 by sodium.wikimedia.org with esmtp (Exim 4.71)
 (envelope-from <daily-article-l-bounces@lists.wikimedia.org>)
 id 1XZC23-0005Mv-4a; Wed, 01 Oct 2014 05:00:55 +0000
content-length: 4394                
message-id:
 <mailman.172107.1412139635.5880.daily-article-l@lists.wikimedia.org>
x-original-to: dam@rightcore.com
delivered-to: dam@rightcore.com
delivered-to: coldspacemachine@gmail.com
x-google-dkim-signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=1e100.net; s=20130820;
 h=delivered-to:x-gm-message-state:dkim-signature:date:mime-version:to
 :message-id:from:subject:precedence:reply-to:list-id
 :list-unsubscribe:list-archive:list-post:list-help:list-subscribe
 :content-type:content-transfer-encoding:sender:errors-to;
 bh=i4mD5uKKyBPm9xney/Om3T/0882TC55AJ/RkI+3TUN4=;
 b=Wfgn9jmOHd6boBYDPrOZKoZwBTggvtWx1kqTg6qeEbrX2OcBASmixPBRPzc0Hshyfd
 ZSWZr0LTNjgqYlFAq3Q08SuZHvoEqFpyvOi0ArM1d/hlHTKZRInWc35Q5nw6ltmapdyt
 IqZYq/FcEmpg1Gfby1JswMv0+7FwSgaI7Fsnc0tgaDvPjhsAEfuZm9LEWVmtyEB/Xl+V
 P/BsilUePS/COiTq4LGJZBeEaWcdAMxmH84LRi0nXuKv3I+ZqN2Pr13H+WSqgOsb96LG
 fG0jacMMIGUDyg12s1Dd+o5hbuFBo9Mc1Sn9FX/tiM7Ysj5OPuIiJdfQAXa2fh7TK607
 HosA==
x-received: by 10.42.27.20 with SMTP id h20mr17768250icc.82.1412139710879;
 Tue, 30 Sep 2014 22:01:50 -0700 (PDT)
x-received: by 10.229.71.73 with SMTP id g9mr66271358qcj.19.1412139709570;
 Tue, 30 Sep 2014 22:01:49 -0700 (PDT)
x-received: by 10.224.38.6 with SMTP id z6mr45541658qad.4.1412139707345;
 Tue, 30 Sep 2014 22:01:47 -0700 (PDT)
x-received: by 10.224.38.6 with SMTP id z6mr45541648qad.4.1412139707252;
 Tue, 30 Sep 2014 22:01:47 -0700 (PDT)
x-received: by 10.194.57.5 with SMTP id e5mr284438wjq.128.1412139616802;
 Tue, 30 Sep 2014 22:00:16 -0700 (PDT)
x-forwarded-to: dam@rightcore.com
x-forwarded-for: coldspacemachine@gmail.com dam@rightcore.com
received-spf: neutral (google.com: 54.208.189.96 is neither permitted nor 
	denied by domain of daily-article-l-bounces@lists.wikimedia.org) 
	client-ip=54.208.189.96;
received-spf: pass (google.com: domain of 
	daily-article-l-bounces@lists.wikimedia.org designates 208.80.154.4 
	as permitted sender) client-ip=208.80.154.4;
authentication-results: mx.google.com;
 spf=neutral (google.com: 54.208.189.96 is neither permitted nor denied by 
	domain of daily-article-l-bounces@lists.wikimedia.org) 
	smtp.mail=daily-article-l-bounces@lists.wikimedia.org;
 dkim=pass header.i=@lists.wikimedia.org;
 dmarc=pass (p=NONE dis=NONE) header.from=lists.wikimedia.org
x-gm-message-state: 
	ALoCoQmYmz3W3KyfbAWHfRxEcLTq4ZWZsxbtFdcuQq6Av0d+9wOdrHF4O+x6l8UY53iOqcKewFNUF8Xj37WX2CVxIeXJgSpfKCNGc2aO6KfOVyvPAHX8XyVpfdCuwrnUS0f57NAWwBYJpEx4OrBwS6viHCgSzfaCCQ==
dkim-signature: v=1; a=rsa-sha256; q=dns/txt; c=relaxed/relaxed; 
	d=lists.wikimedia.org; s=wikimedia;
 h=Sender:Content-Transfer-Encoding:Content-Type:List-Subscribe:List-Help:List-Post:List-Archive:List-Unsubscribe:List-Id:Reply-To:Subject:From:Message-ID:To:MIME-Version:Date;
	bh=i4mD5uKKyBPm9xney/Om3T/0882TC55AJ/RkI+3TUN4=;
 b=hgo8iHFt+CQyWSRV2/IYc4wP2O71fVNRx6bYgvG8GztklZUkeBuang+uqvQrTKo7oH3DS6IcQQzgrUdNxcL75/gG02edtI7wysYeM2FvdivgrGHQhTsHxyhInCLyHUfwlBOAepIDrnmEhRXYn5yCMtvBH6fro+Rrp83zhTvvNCc=;
mime-version: 1.0
x-beenthere: daily-article-l@lists.wikimedia.org
x-mailman-version: 2.1.13
precedence: list
reply-to: dal-feedback@wikimedia.org
list-id: English Wikipedia Article of the Day
 <daily-article-l.lists.wikimedia.org>
list-unsubscribe: 
	<https://lists.wikimedia.org/mailman/options/daily-article-l>,
 <mailto:daily-article-l-request@lists.wikimedia.org?subject=unsubscribe>
list-archive: <https://lists.wikimedia.org/pipermail/daily-article-l>
list-post: <mailto:daily-article-l@lists.wikimedia.org>
list-help: <mailto:daily-article-l-request@lists.wikimedia.org?subject=help>
list-subscribe: 
	<https://lists.wikimedia.org/mailman/listinfo/daily-article-l>,
 <mailto:daily-article-l-request@lists.wikimedia.org?subject=subscribe>
content-type: text/plain; charset="utf-8"
x-content-transfer-encoding: base64
x-sender: daily-article-l-bounces@lists.wikimedia.org
errors-to: daily-article-l-bounces@lists.wikimedia.org
x-bayes-prob: 0.9999 (Score 3, tokens from: morano@computer.org, CS_DEFAULT,
	80_DEFAULT, default, @@RPTN)
x-spam-score: 3.00 (***) [Hold at 6.00] SPF(pass:0),Bayes(0.9999:3.0)
x-canit-geo: ip=208.80.154.4; country=US; region=California; city=San 
	Francisco; latitude=37.7749; longitude=-122.4194; 
	http://maps.google.com/maps?q=37.7749,-122.4194&z=6
x-canitpro-stream: morano@computer.org (inherits from 
	CS_DEFAULT,80_DEFAULT,default)
x-canit-stats-id: Bayes signature not available
x-scanned-by: CanIt [cuce . ieee . org] on 10.144.194.218
x-bogosity: Unsure, tests=bogofilter, spamicity=0.520000, version=1.1.5
status:           
From:  English Wikipedia Article of the Day
 <daily-article-l@lists.wikimedia.org>
x-To:  daily-article-l@lists.wikimedia.org
x-Date: Tue, 30 Sep 2014 22:00:16 -0700 (PDT)
x-Subject: [Daily article] October 1: Mucho Macho Man

TXVjaG8gTWFjaG8gTWFuIChmb2FsZWQgMjAwOCkgaXMgYSByZXRpcmVkIEFtZXJpY2FuIFRob3Jv
dWdoYnJlZApyYWNlaG9yc2Ugd2hvIHdvbiB0aGUgMjAxMyBCcmVlZGVycycgQ3VwIENsYXNzaWMu
IE5hbWVkIGFmdGVyIHRoZQpWaWxsYWdlIFBlb3BsZSBzb25nICJNYWNobyBNYW4iLCBoZSB3YXMg
b3duZWQgZm9yIG1vc3Qgb2YgaGlzIHJhY2luZwpjYXJlZXIgYnkgRGVhbiBhbmQgUGF0dGkgUmVl
dmVzIG9mIFJlZXZlcyBUaG9yb3VnaGJyZWQgUmFjaW5nLCBhbmQKdHJhaW5lZCBieSBLYXRoeSBS
aXR2by4gQm9ybiBsYXRlIGluIHRoZSB5ZWFyIGZvciBhIFRob3JvdWdoYnJlZCBmb2FsLAphcyBh
IGdyb3dpbmcgdHdvLSBhbmQgdGhyZWUteWVhci1vbGQgaGUgaGFkIHRvIGNvbXBldGUgYWdhaW5z
dCBob3JzZXMKdGhhdCB3ZXJlIHNldmVyYWwgbW9udGhzIG9sZGVyLiBJbiAyMDExLCBoZSBjb21w
ZXRlZCBpbiBhbGwgdGhyZWUgVHJpcGxlCkNyb3duIHJhY2VzLCBmaW5pc2hpbmcgdGhpcmQgaW4g
dGhlIEtlbnR1Y2t5IERlcmJ5LiBBZnRlciBzdXJnZXJ5IGluCjIwMTEgdG8gYWRkcmVzcyBhIGJy
ZWF0aGluZyBwcm9ibGVtLCBoZSBjYW1lIGJhY2sgYW5kIHdvbiB0aHJlZSBncmFkZWQKc3Rha2Vz
IHJhY2VzIGFuZCBmaW5pc2hlZCBhIGNsb3NlIHNlY29uZCBpbiB0aGUgMjAxMiBCcmVlZGVycycg
Q3VwCkNsYXNzaWMuIFdpbm5pbmcgdGhhdCByYWNlIGluIDIwMTMgKHBpY3R1cmVkIHdpdGggam9j
a2V5IEdhcnkgU3RldmVucyksCmNvbWJpbmVkIHdpdGggdGhlIGNvbXBlbGxpbmcgaHVtYW4taW50
ZXJlc3Qgc3RvcmllcyBzdXJyb3VuZGluZyB0aGUKaG9yc2UgYW5kIHRoZSBwZW9wbGUgd2hvIHdv
cmtlZCB3aXRoIGhpbSwgZWFybmVkIGhpbSB0aGUgU2VjcmV0YXJpYXQgVm94ClBvcHVsaSBBd2Fy
ZCBhbmQgdGhlIE5hdGlvbmFsIFRob3JvdWdoYnJlZCBSYWNpbmcgQXNzb2NpYXRpb24gTW9tZW50
IG9mCnRoZSBZZWFyLiBNdWNobyBNYWNobyBNYW4gd29uIGFub3RoZXIgcmFjZSBpbiBKYW51YXJ5
IDIwMTQgYW5kIHdhcwpyZXRpcmVkIGluIEp1bHkgZHVlIHRvICJ3ZWFyIGFuZCB0ZWFyIiwgYnV0
IGVzc2VudGlhbGx5IHNvdW5kLiBIZSBub3cKbGl2ZXMgYXQgQWRlbmEgU3ByaW5ncyB3aGVyZSBo
ZSB3aWxsIHN0YW5kIGF0IHN0dWQuCgpSZWFkIG1vcmU6IDxodHRwczovL2VuLndpa2lwZWRpYS5v
cmcvd2lraS9NdWNob19NYWNob19NYW4+CgpfX19fX19fX19fX19fX19fX19fX19fX19fX19fX19f
ClRvZGF5J3Mgc2VsZWN0ZWQgYW5uaXZlcnNhcmllczoKCjE4OTE6CgpTdGFuZm9yZCBVbml2ZXJz
aXR5LCBmb3VuZGVkIGJ5IHJhaWxyb2FkIG1hZ25hdGUgYW5kIENhbGlmb3JuaWEKR292ZXJub3Ig
TGVsYW5kIFN0YW5mb3JkIGFuZCBoaXMgd2lmZSBKYW5lIG9uIHRoZWlyIGZvcm1lciBmYXJtIGxh
bmRzIGluClBhbG8gQWx0bywgQ2FsaWZvcm5pYSwgb2ZmaWNpYWxseSBvcGVuZWQgd2l0aCA1NTkg
c3R1ZGVudHMgYW5kIGZyZWUKdHVpdGlvbi4KPGh0dHBzOi8vZW4ud2lraXBlZGlhLm9yZy93aWtp
L1N0YW5mb3JkX1VuaXZlcnNpdHk+CgoxOTEwOgoKQSBsYXJnZSBib21iIGRlc3Ryb3llZCB0aGUg
TG9zIEFuZ2VsZXMgVGltZXMgYnVpbGRpbmcgaW4gTG9zCkFuZ2VsZXMsIGtpbGxpbmcgMjEgcGVv
cGxlLgo8aHR0cHM6Ly9lbi53aWtpcGVkaWEub3JnL3dpa2kvTG9zX0FuZ2VsZXNfVGltZXNfYm9t
YmluZz4KCjE5NDk6CgpDaGluZXNlIENvbW11bmlzdCBQYXJ0eSBsZWFkZXIgTWFvIFplZG9uZyBw
cm9jbGFpbWVkIHRoZQplc3RhYmxpc2htZW50IG9mIHRoZSBQZW9wbGUncyBSZXB1YmxpYyBvZiBD
aGluYS4KPGh0dHBzOi8vZW4ud2lraXBlZGlhLm9yZy93aWtpL0NoaW5hPgoKMTk2NDoKClRoZSBU
xY1rYWlkxY0gU2hpbmthbnNlbiwgdGhlIGZpcnN0IFNoaW5rYW5zZW4gbGluZSBvZiBoaWdoLQpz
cGVlZCByYWlsd2F5cyBpbiBKYXBhbiwgb3BlbmVkIGZvciBzZXJ2aWNlLgo8aHR0cHM6Ly9lbi53
aWtpcGVkaWEub3JnL3dpa2kvVCVDNSU4RGthaWQlQzUlOERfU2hpbmthbnNlbj4KCjE5OTE6CgpO
ZXcgWmVhbGFuZCdzIFJlc291cmNlIE1hbmFnZW1lbnQgQWN0IGNhbWUgaW50byBlZmZlY3QsCnJl
Z3VsYXRpbmcgYWNjZXNzIHRvIG5hdHVyYWwgYW5kIHBoeXNpY2FsIHJlc291cmNlcyBzdWNoIGFz
IGxhbmQsIGFpcgphbmQgd2F0ZXIsIHRvIGVuc3VyZSB0aGVpciBzdXN0YWluYWJsZSB1c2UuCjxo
dHRwczovL2VuLndpa2lwZWRpYS5vcmcvd2lraS9SZXNvdXJjZV9NYW5hZ2VtZW50X0FjdF8xOTkx
PgoKX19fX19fX19fX19fX19fX19fX19fX19fX19fX18KV2lrdGlvbmFyeSdzIHdvcmQgb2YgdGhl
IGRheToKCndoZXJlbm90OgpPdGhlciByZWxhdGVkIHBsYWNlcywgd2hlcmV2ZXI7IGFzIGluLCB0
byBCYWdkYWQsIENoaW5hLCBhbmQgd2hlcmVub3QuCjxodHRwczovL2VuLndpa3Rpb25hcnkub3Jn
L3dpa2kvd2hlcmVub3Q+CgpfX19fX19fX19fX19fX19fX19fX19fX19fX18KV2lraXF1b3RlIHF1
b3RlIG9mIHRoZSBkYXk6CgrCoCAgICAgIFdlIGxpdmUgaW4gYSB0aW1lIG9mIHRyYW5zaXRpb24s
IGFuIHVuZWFzeSBlcmEgd2hpY2ggaXMgbGlrZWx5CnRvIGVuZHVyZSBmb3IgdGhlIHJlc3Qgb2Yg
dGhpcyBjZW50dXJ5LiBEdXJpbmcgdGhlIHBlcmlvZCB3ZSBtYXkgYmUKdGVtcHRlZCB0byBhYmFu
ZG9uIHNvbWUgb2YgdGhlIHRpbWUtaG9ub3JlZCBwcmluY2lwbGVzIGFuZCBjb21taXRtZW50cwp3
aGljaCBoYXZlIGJlZW4gcHJvdmVuIGR1cmluZyB0aGUgZGlmZmljdWx0IHRpbWVzIG9mIHBhc3Qg
Z2VuZXJhdGlvbnMuCldlIG11c3QgbmV2ZXIgeWllbGQgdG8gdGhpcyB0ZW1wdGF0aW9uLiBPdXIg
QW1lcmljYW4gdmFsdWVzIGFyZSBub3QKbHV4dXJpZXMsIGJ1dCBuZWNlc3NpdGllcyDigJQgbm90
IHRoZSBzYWx0IGluIG91ciBicmVhZCwgYnV0IHRoZSBicmVhZAppdHNlbGYuICAgICAgIMKgCiAg
LS1KaW1teSBDYXJ0ZXIKPGh0dHBzOi8vZW4ud2lraXF1b3RlLm9yZy93aWtpL0ppbW15X0NhcnRl
cj4KCl9fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fX19fCldpa2lw
ZWRpYSBEYWlseSBBcnRpY2xlIG1haWxpbmcgbGlzdC4KVG8gdW5zdWJzY3JpYmUsIHZpc2l0Ogpo
dHRwczovL2xpc3RzLndpa2ltZWRpYS5vcmcvbWFpbG1hbi9saXN0aW5mby9kYWlseS1hcnRpY2xl
LWwKUXVlc3Rpb25zIG9yIGNvbW1lbnRzPyBDb250YWN0IGRhbC1mZWVkYmFja0B3aWtpbWVkaWEu
b3Jn
