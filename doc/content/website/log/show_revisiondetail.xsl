<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt" 
	exclude-result-prefixes="xsl msxsl"
>

	<xsl:param name="revision" />

	<xsl:output method="html" indent="no" standalone="yes" omit-xml-declaration="yes" />

	<xsl:template match="log">
			<b>
				<xsl:value-of select="logentry/author" />, 
				rev <xsl:value-of select="$revision" />, </b> 
			<xsl:value-of select="logentry/date" /> (<a href="javascript:void(0);" onclick="show_revision('{$revision}')">details</a>)<br />
			<pre>
<xsl:value-of select="logentry/msg" /></pre>
			<ul style="display:none">
				<xsl:for-each select="logentry/paths/path">
					<li><a href="#"><xsl:value-of select="text()" /></a></li>
				</xsl:for-each>
			</ul>
	</xsl:template>
	

</xsl:stylesheet>

