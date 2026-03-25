# Shortcut System Specification

## Overview

Shortcut System, kullanıcıların uygulamayı fare kullanmadan hızlı ve akıcı şekilde kullanabilmesini sağlayan bir verimlilik katmanıdır.  
Bu sistemin amacı, kullanıcıların notlara ve dosya yollarına saniyeler içinde erişebilmesini sağlamak, uygulama içi navigasyonu hızlandırmak ve günlük kullanımda gereksiz adımları ortadan kaldırmaktır.

Shortcut sistemi üç ana bileşenden oluşur:

1. Global Quick Launch (Hızlı Başlat Paneli)
2. Uygulama İçi Navigasyon Kısayolları
3. Drag & Drop ile Otomatik Yol Ekleme

Bu özellikler birlikte çalışarak uygulamanın kullanımını daha akıcı, hızlı ve modern hale getirir.

---

# 1. Global Quick Launch

## Tanım

Global Quick Launch, uygulama arka planda çalışırken bile erişilebilen hızlı bir arama ve başlatma panelidir.  
Bu sistem, modern işletim sistemlerindeki **Spotlight**, **PowerToys Run** veya **Raycast** benzeri bir davranış sergiler.

Kullanıcı belirlenen bir klavye kombinasyonuna bastığında ekranın ortasında küçük ve şık bir arama paneli açılır.

## Açılma Davranışı

Kullanıcı aşağıdaki kombinasyonlardan biri ile paneli açabilir:

Alt + Space  
veya  
Ctrl + Shift + K

Bu kombinasyon tetiklendiğinde:

- Ekranın ortasında minimal bir arama barı görünür
- Arama kutusu otomatik olarak odaklanmış olur
- Kullanıcı direkt yazmaya başlayabilir

## Arama İşlevi

Kullanıcı arama kutusuna yazmaya başladığında sistem şu verileri tarar:

- kayıtlı notlar
- kayıtlı dosya yolları
- takma adlar (alias)
- daha önce açılmış öğeler

Sonuçlar yazdıkça anında filtrelenir.

## Sonuç Davranışı

Arama sonuçları aşağıdaki şekilde listelenir:

- en alakalı sonuç en üstte
- notlar ve yollar görsel ikonlarla ayrılır
- klavye ile yukarı aşağı gezilebilir

Kullanıcı:

Enter tuşuna bastığında seçili öğe direkt açılır.

Bu sayede kullanıcı fare kullanmadan dosya veya notlara erişebilir.

## Kapanma Davranışı

Panel aşağıdaki durumlarda kapanır:

- Escape tuşuna basıldığında
- kullanıcı bir öğe açtığında
- kullanıcı başka bir pencereye odaklandığında

---

# 2. Application Navigation Shortcuts

## Tanım

Uygulama içerisinde kullanıcıların menüler arasında hızlı şekilde geçebilmesini sağlayan standart klavye kısayolları bulunur.

Bu sistem fare kullanımını azaltarak üretkenliği artırmayı amaçlar.

## Standart Kısayollar

### Yeni Not

Ctrl + N

Bu kısayol kullanıldığında:

- "Yeni Not" sayfası açılır
- başlık alanı otomatik odaklanır
- kullanıcı yazmaya hemen başlayabilir

---

### Yeni Yol (Link) Ekleme

Ctrl + L

Bu kısayol kullanıldığında:

- "Yol Ekle" formu açılır
- dosya yolu alanı otomatik odaklanır
- kullanıcı yeni bir yol ekleyebilir

---

### Arama Başlatma

Ctrl + F

Bu kısayol uygulama içindeki arama sistemini başlatır.

Arama şu alanları kapsar:

- kayıtlı notlar
- kayıtlı dosya yolları
- takma adlar

Arama alanı açıldığında imleç otomatik olarak arama kutusuna yerleşir.

---

### Sekmeler Arası Geçiş

Ctrl + Tab

Bu kısayol yan menüdeki ana sekmeler arasında geçiş yapar.

Örnek geçiş sırası:

Yollar → Notlar → tekrar Yollar

Bu sayede kullanıcı fare kullanmadan ana bölümler arasında dolaşabilir.

---

# 3. Drag & Drop Path Creation

## Tanım

Drag & Drop sistemi, kullanıcıların dosyaları sürükleyerek uygulamaya ekleyebilmesini sağlar.

Bu özellik özellikle hızlı yol eklemek isteyen kullanıcılar için büyük bir zaman kazancı sağlar.

## Temel Davranış

Kullanıcı bilgisayarından bir dosyayı veya klasörü uygulamaya sürükleyip bıraktığında sistem otomatik olarak:

- "Yol Ekle" formunu açar
- dosya yolunu otomatik doldurur
- dosya adını başlık alanına öneri olarak yazar

Kullanıcı yalnızca gerekli düzenlemeleri yapıp kaydedebilir.

## Sürükleme Alanı

Dosya şu alanlara bırakılabilir:

- uygulama ana penceresi
- yol listesi alanı
- boş alanlar

Sistem bırakılan dosyayı algılar ve yol oluşturma sürecini başlatır.

## Kullanıcı Geri Bildirimi

Dosya sürüklenirken arayüz kullanıcıya görsel geri bildirim verir.

Örneğin:

- sürükleme alanı hafif şekilde vurgulanır
- "Dosyayı bırakın" mesajı gösterilir

Dosya bırakıldığında yol ekleme formu açılır.

---

# User Experience Goals

Shortcut sistemi aşağıdaki hedefleri sağlar:

- Fare kullanımını azaltmak
- Hızlı erişim sağlamak
- Minimum adımla işlem yapmak
- Profesyonel üretkenlik araçlarına benzer deneyim sunmak

Bu sistem sayesinde kullanıcılar notlarına ve dosya yollarına birkaç saniye içinde erişebilir.

---

# Future Expansion

Shortcut sistemi ileride şu özelliklerle genişletilebilir:

- Quick Launch üzerinden not oluşturma
- Quick Launch üzerinden yol ekleme
- Quick Launch komut sistemi (örn: "new note")
- kullanıcıya özel kısayol atamaları
- son kullanılan öğeler listesi

# Ek Notlar:

Side bara  Ayalarlar sekmesi ekle buradaki bütün kısayollar buradan ayarlanabilsin. 


Kısayollar nasıl ayarlanacak;

Canlı olarak bastığımız tuşlar kaydedilebilecek. 

Kaydediledikten sonra tuş aşamalarını (baş Çek/ Basılı Tut / Şu kadar Bas [X]) ayarlayabileceğiz

Bu sayfa da karanlık temadan veya aydınlık temadan etkilenebilecek.