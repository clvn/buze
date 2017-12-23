<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt" 
	exclude-result-prefixes="xsl msxsl"
>

	<xsl:param name="methodName" />

	<xsl:output method="html" indent="no" standalone="yes" omit-xml-declaration="yes" />

	<xsl:template match="zidl">
		<div class="method">
			<xsl:apply-templates select="namespace/class/method[name=$methodName]" />
		</div>
	</xsl:template>

	<xsl:template match="method">
		<h2><xsl:value-of select="name" /></h2>
		<xsl:apply-templates select="description[1]" />
		
		<div class="signature">
			<xsl:apply-templates select="returns" mode="type" />&#160;<b><xsl:value-of select="name"/></b>(
			<xsl:if test="arguments/argument">
				<br />
				<div style="margin-left:20px">
					<xsl:for-each select="arguments/argument">
						<xsl:apply-templates select="type" mode="type" />&#160;<xsl:value-of select="name" />
						<xsl:if test="position() != last()">,<br /> </xsl:if>
					</xsl:for-each>
				</div>
			</xsl:if>
			);
		</div>
		
		<xsl:apply-templates select="description[position() &gt; 1]" />
		<p>
			<a href="javascript:renderClass('{../name}')">Back</a>
		</p>
	</xsl:template>
	
	<xsl:template match="description">
		<p>
			<xsl:value-of select="." />
		</p>
	</xsl:template>

	<xsl:template match="text()" mode="type">
		<xsl:choose>
			<xsl:when test="substring(., 0, string-length(.) - 2) = //class/name">
				<a href="javascript:renderClass('{substring(., 0, string-length(.) - 2)}')"><xsl:value-of select="." /></a>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="." />
			</xsl:otherwise>
			<!--xsl:when test=".='int' or .='int*' or .='unsigned int' or .= 'unsigned int*' or .='void' or .='void*' or .='char*' or .='const char*' or .='char' or .='unsigned char' or .='unsigned short' or .='short'">
				<xsl:value-of select="." />
			</xsl:when>
			<xsl:otherwise>
				<a href="javascript:renderClass('{substring(., 0, string-length(.) - 2)}')"><xsl:value-of select="." /></a>
			</xsl:otherwise-->
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>

