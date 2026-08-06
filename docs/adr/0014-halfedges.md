# 0014 - Halfedges 

# Durum 
Elimizdeki vertices/indices sistemi modellerin grafik kartında gösterilmesi için gayet yeterli. Fakat modelleri editlemek istediğimizde bu yapı oldukça kullanışsız. Bizim topolojik olarak editleme yapabileceğimiz bir sisteme ihtiyacımız var.  


# Karar
Halfedge veri yapısını kullanmak. 

## Neden Halfedge? 
- Başlangıç için basit ve öğrenmesi kolay. 
- Topology correct (doğal olarak manifold oluşturuyor) 
- Programa eklemek istediğim işlemler için yeterli 

## Nasıl çalışacak
Ekstradan bir component ekleyeceğiz. Bu component mesh'imizi topolojik olarak tutacak. Ihtiyaç halinde ekrana bastırmak için "construct()" isimli bir fonksiyon kullanarak Mesh üretip RenderComponent üzerinden ekrana basacak. 

Bu faz-1 için yeterli olacaktır. Fakat ilerleyen dönemlerde bu yöntemi daha optimize hale getirmemiz lazım. Çünkü bir vertex'in pozisyonu değiştiğinde tüm mesh i yeniden oluşturmak ciddi anlamda verimsiz. Fakat faz-1 de bunu göz ardı etmemiz lazım. Ana amacımız mesh operasyonlarını doğru bir şekilde yapmak. 

## Hangi Mesh operasyonları olacak 

- [+] edge flip: Verilen edge, eğer iki tris arasında ise edge i saat yönünde(?) 1 vertex döndürür. 
- [+] edge split: Verilen edge'i ikiye böler. Yani edge in ortasına 1 vertex eklenir. 

- [~] edge collapse: Verilen edge'i siler. Daha doğrusu iki kenarını bir noktada birleştirir ve gerekirse yok olan face/halfedge/vertex leri kaldırır.

- [-] extrude:
- [-] insert: 
- [-] revolve:

# Alternatifler
Alternatifleri yeterince araştırmadım. 


# Pozitif Çıktılar
- Elimizde mesh editlemek için güzel bir yapımız oldu. 
- Mokalab, bir viewer'dan çıkıp editöre dönüşmeye başladı. 

# Negatif Çıktılar
Mimarimizi tasarlarken böyle bir yapı ekleyeceğimizi bilmiyorduk. O sebeple var olan yapımızda bazı köklü değişiklikler olabilir. Burası zaman gösterecek