<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt" 
	exclude-result-prefixes="xsl msxsl"
>

	<xsl:param name="revision" />

	<xsl:output method="html" indent="no" standalone="yes" omit-xml-declaration="yes" />

	<xsl:template match="zidl">
		<div class="namespace">
			<xsl:apply-templates select="namespace" />
		</div>
	</xsl:template>

	<xsl:template match="namespace">
		<h2><xsl:value-of select="dlname" /></h2>
		<ul>
			<xsl:apply-templates select="class[count(method)&gt;0]">
				<xsl:sort select="name" />
			</xsl:apply-templates>
		</ul>
		<a href="javascript:renderClass('')">View all methods</a>
		<hr />
		<h2>Other classes</h2>
		<ul>
			<xsl:apply-templates select="class[count(method)=0]">
				<xsl:sort select="name" />
			</xsl:apply-templates>
		</ul>

	</xsl:template>

	<xsl:template match="class">
		<li>
			<a href="javascript:renderClass('{name}')"><xsl:value-of select="name" /></a>
		</li>
	</xsl:template>

</xsl:stylesheet>

