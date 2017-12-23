<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt" 
	exclude-result-prefixes="xsl msxsl"
>

	<xsl:param name="className" />

	<xsl:output method="html" indent="no" standalone="yes" omit-xml-declaration="yes" />

	<xsl:template match="zidl">
		<div class="class">
			<xsl:apply-templates select="namespace" />
		</div>

	</xsl:template>

	<xsl:template match="namespace">
		<xsl:apply-templates select="class[(name=$className or $className='')]">
			<xsl:sort select="name" />
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match="description">
		<p>
			<xsl:value-of select="." />
		</p>
	</xsl:template>

	<xsl:template match="class">
		<h2><xsl:value-of select="name" /></h2>
		<xsl:apply-templates select="description" />
		<!--p>
			<xsl:value-of select="description" />
		</p-->
		
		<!--xsl:if test="class">
			<h2>Subclasses</h2>
			<xsl:apply-templates select="class">
				<xsl:sort select="name" />
			</xsl:apply-templates>
		</xsl:if-->

		<table border="1" width="100%" style="margin-bottom:16px">
			<tr>
				<td colspan="2" valign="top">
					<b><xsl:value-of select="name" /> members:</b>
				</td>
			</tr>
			<xsl:apply-templates select="method" />
		</table>
	</xsl:template>
	
	<xsl:template match="method">
		<tr>
			<td width="50%" valign="top"><a href="javascript:renderMethod('{name}')"><xsl:value-of select="name" /></a></td> 
			<td width="50%" valign="top">
				<xsl:value-of select="description" />
			</td>
		</tr>
	</xsl:template>

</xsl:stylesheet>

