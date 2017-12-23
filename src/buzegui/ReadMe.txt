

buzegui implementerer kun en ekstern funksjon: GetViewLibrary()

buzegui skal kun bruke api'er i buze.zidl for å interface med buze.

buzegui skal bli en dll som implementerer basic views i buze.

buzegui skal loades av buzelib.dll, når den enumerater viewene og adder dem på mainframen.

alt av gui-kode og resources fra buzelib skal inn her, unntatt det som hører til mainframen.

buzelib skal kun bestå av implementasjonen av c-interfacet.

når 
	1) buzelib kun har c-interface-implementasjon, og 
	2) buzegui har implementert alle views mot c-apiet
	3) buze har shellet til mainframen med docktabframe

først da kan vi endre buzelib og buzegui til dll'er. 

i mellomtiden vil det være krysskoblinger som krever at buzelib og buzegui er static libs
