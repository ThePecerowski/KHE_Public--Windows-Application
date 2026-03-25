Kişisel Hızlı Erişim Sistemi;

KHE Private Uygulaması

Bu uygulama nın amacı;

Uygulamanın amacı, önemli dosyaların yollarını tutmak, veya önemli bilgileri içerisinde not olarak erişilebilir bir halde tutmak. Bu uygulama bir windows uygulaması olacak. 

Kullanılacaklar;
Dil;
C++

Tasarım;

İki farklı tema olacak;

Aydınlık tema;

Renkler; Temel yüzeyler:

Background: #FFFFFF
Surface: #F5F7FA
Border: #E4E7EC

Metinler:

Primary text: #111111
Secondary text: #667085

Accent renkler:

Primary Blue: #3B82F6
Light Blue: #93C5FD
Blue Gradient:
linear-gradient(135deg, #3B82F6, #6366F1)

Secondary accent:

Purple: #8B5CF6

Durumlar:

Hover: biraz koyu mavi
Active: gradient
Focus: açık mavi

Karanlık tema;

Renkler; Temel yüzeyler:

Background: #0F172A
Surface: #1E293B
Card: #1F2937
Border: #334155

Metin:

Primary text: #F8FAFC
Secondary text: #94A3B8

Accent renkler:

Primary Blue: #60A5FA
Light Blue: #93C5FD

Gradient:

linear-gradient(135deg, #60A5FA, #8B5CF6)

Secondary accent:

Purple: #A78BFA

3️⃣ Light → Dark dönüşüm mantığı

Profesyonel UI'larda şu mantık kullanılır:

Element	Light	Dark
Background	Beyaz	Çok koyu mavi
Surface	Açık gri	Koyu gri
Text	Siyah	Beyaz
Accent	Normal mavi	Daha parlak mavi

Uygulama içerisinde olacaklar;

1. Dosya yolu kaydetme (Sadece dosya yolunu kaydeder yolun üzerine tıklandığında o dosya yolunu açar eğer iconun üzerine tıklanırsa direkt o dosyayı açar.)

2. Not kaydetme. Notları Rich Text halinde kaydedile bilir. Notlar düzenlenebilir silinebilir olacak. Ve de bunlar kaydedilecek. Aynı zamanda notlar a resim ve de video, ses eklenebilir. 

3. Bir server olmayacak ama veriler SQL yolu ile kaydedilecek. 

Menüler;

1. Ana Menü;

Sol taraf Side bar;  burada Kaydedilen Yollar ve Notlar olacak. bağlantıları olacak.

Ana menü içeriği; En üstte en son kullanılan notlar ve onun altın da da en son kullanılan yollar olacak. 

Hemen altında ise hızlı işlemler kısmı olacak. 

Hızlı yol ekle
Hızlı Not ekle

Yol Ekle sayfası;

İçeriğinde şunlar olacak; 

bir tablo olacak bu tablo da eklenen bütün yollar olacak. 

bu yollar liste halinde listelenecek liste elemanları üzeirinde, düzenle ve sil, dosya yoluna git, doyayı aç buttonları olacak. 

bu tablo nun üstünde ise yeni dosya yolu ekle butonu olacak.

liste elemanlarını üzerinde ki yola tıklandığında direkt o dosya yolunu açacak.

Eğer elemanın üzerinde ki dosya iconuna tıklanır ise direkt o dosyayı veya programı açacak.

Nasıl Yol eklenecek;

Yol ekleme formunda iki türlü yol eklenebiklecke bu yol ekleme türleri pencereler ile yönetilebilecek. 

Ekleme türleri;

1. Bir dosya veya program seçerek onun yolunu ekle. 
2. Yolu el ile girerek o yolu ekle.


Not Ekle sayfası;

İçeriğinde şunlar olacak;

Bütün notlar kısmı olack burad notlar card halinde olacaklar. 

cardlar üzerinde düzenle ve sil işlemi yapılabilecek. 

Cardların üzerlerine tıklandığında içerik sayfasına gidilecek. 

bu içerik sayfasında notun içeriği görüntülenebilecek, video var ise video ve ses oynatılabilecek. 

UX eklemeler;

Ux içerisinde şunlar olack;

üst tarafta küçük bir çizgi halinde yol izleyicisi 







