<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt" 
	exclude-result-prefixes="xsl msxsl"
>

	<xsl:param name="count" />

	<xsl:output method="html" indent="no" standalone="yes" omit-xml-declaration="yes" />

	<xsl:template match="last">

		<xsl:variable name="head_revision"><xsl:value-of select="text()" /></xsl:variable>

		<div>
			<xsl:call-template name="show-revisions-recursively">
				<xsl:with-param name="revision" select="$head_revision" />
				<xsl:with-param name="head_revision" select="$head_revision" />
				<xsl:with-param name="counter" select="1" />
			</xsl:call-template>
		</div>

	</xsl:template>

	<xsl:template name="show-revisions-recursively">
		<xsl:param name="revision" />
		<xsl:param name="head_revision" />
		<xsl:param name="counter" />
		<xsl:variable name="revision_filename"><xsl:value-of select="$revision" />.xml</xsl:variable>
		<xsl:variable name="revision_file" select="document($revision_filename)" />
		<div style="margin-bottom:8px" rev="{$revision}" src="{$revision_filename}" class="rev">
		</div>

		<xsl:if test="$counter &lt; $count">
			<xsl:call-template name="show-revisions-recursively">
				<xsl:with-param name="revision" select="$revision - 1" />
				<xsl:with-param name="head_revision" select="$head_revision" />
				<xsl:with-param name="counter" select="$counter + 1" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>

