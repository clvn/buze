

buzegui implementerer kun en ekstern funksjon: GetViewLibrary()

buzegui skal kun bruke api'er i buze.zidl for � interface med buze.

buzegui skal bli en dll som implementerer basic views i buze.

buzegui skal loades av buzelib.dll, n�r den enumerater viewene og adder dem p� mainframen.

alt av gui-kode og resources fra buzelib skal inn her, unntatt det som h�rer til mainframen.

buzelib skal kun best� av implementasjonen av c-interfacet.

n�r 
	1) buzelib kun har c-interface-implementasjon, og 
	2) buzegui har implementert alle views mot c-apiet
	3) buze har shellet til mainframen med docktabframe

f�rst da kan vi endre buzelib og buzegui til dll'er. 

i mellomtiden vil det v�re krysskoblinger som krever at buzelib og buzegui er static libs
