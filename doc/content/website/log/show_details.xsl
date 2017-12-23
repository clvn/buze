<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt" 
	exclude-result-prefixes="xsl msxsl"
>

	<xsl:param name="revision" />

	<xsl:output method="html" indent="no" standalone="yes" omit-xml-declaration="yes" />

	<xsl:template match="last">
		<xsl:variable name="revision_filename"><xsl:value-of select="$revision" />.xml</xsl:variable>
		<xsl:variable name="revision_file" select="document($revision_filename)" />

		<xsl:variable name="head_revision"><xsl:value-of select="text()" /></xsl:variable>

		<div>
			<p>
			<b>Revision <xsl:value-of select="$revision" />. Commited by <xsl:value-of select="$revision_file/log/logentry/author" />.</b><br />
			<xsl:value-of select="$revision_file/log/logentry/msg" />
			</p>
			<ul>
			<xsl:for-each select="$revision_file/log/logentry/paths/path">
				<li>(<xsl:value-of select="@action" />) <xsl:value-of select="text()" /></li>
			</xsl:for-each>
			</ul>
			<iframe src="diff-{$revision}.txt" width="100%" height="275px"></iframe>
			<p>
				<a href="javascript:void(0)" onclick="render_revisions()">Back</a>
			</p>
		</div>

	</xsl:template>

</xsl:stylesheet>

