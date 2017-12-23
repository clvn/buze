<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" version="4.0" indent="yes" omit-xml-declaration="yes" encoding="utf-8" standalone="yes" />

<xsl:param name="BaseName" />
<xsl:param name="RootPath" />

<xsl:template match="/">

	<html>
		<head>
			<link rel="stylesheet" type="text/css" href="{$RootPath}assets/style.css" />
			<xsl:copy-of select="/html/head/*|/html/head/text()" />
			<xsl:choose>
				<xsl:when test="not(/html/head/title) and /html/body/h1[1]">
					<title><xsl:value-of select="/html/body/h1[1]" /></title>
				</xsl:when>
				<xsl:otherwise>
					<title><xsl:value-of select="$BaseName" /></title>
				</xsl:otherwise>
			</xsl:choose>
			<!--xsl:if test="not(/html/head/title)">
				<title><xsl:value-of select="$BaseName" /></title>
			</xsl:if-->

		</head>
		<xsl:copy-of select="/html/body/*|/html/body/text()" />
	</html>
	
</xsl:template>

</xsl:stylesheet>

