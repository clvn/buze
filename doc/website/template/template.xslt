<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" version="4.0" indent="yes" omit-xml-declaration="yes" encoding="utf-8" standalone="yes" />

<xsl:param name="BaseName" />
<xsl:param name="RootPath" />
<xsl:param name="SourcePath" />

<xsl:template match="/">

	<html>
		<head>
			<link rel="stylesheet" type="text/css" href="{$RootPath}assets/style.css" />
			<xsl:copy-of select="/html/head/*|/html/head/text()" />
			<xsl:if test="not(/html/head/title)">
				<title>Buzé - <xsl:value-of select="$BaseName" /></title>
			</xsl:if>
		</head>
		<body>
			<div id="apDiv2">
		
				<div id="apDiv4">
					<a href="{$RootPath}index.html">Home</a> | 
					<a href="https://sourceforge.net/u/anders-e/buze/discussion/general/">Forum</a> |
					<a href="{$RootPath}user/index.html">User Documentation</a> | 
					<a href="{$RootPath}developer/index.html">Developer Documentation</a>
				</div>
	
				<div id="apDiv1">
					<!--xsl:copy-of select="html/body/*|html/body/text()" /-->
					<xsl:apply-templates select="html/body/*|html/body/text()" />
				</div>
			</div>
		</body>
	</html>
	
</xsl:template>

<xsl:template match="rss">
	<!--RSS HERE <xsl:value-of select="concat($SourcePath, @src)" /-->
	<xsl:apply-templates select="document(concat($SourcePath, @src))" mode="rss"/>
</xsl:template>

<xsl:template match="@*|node()|text()">
	<xsl:copy>
		<xsl:apply-templates select="@*|node()|text()" />
	</xsl:copy>
</xsl:template>

<xsl:template match="/" mode="rss">
	<xsl:apply-templates select="rss/channel/item" mode="rss" />
</xsl:template>

<xsl:template match="rss/channel/item" mode="rss">
	<div style="margin-bottom:8px; padding-bottom:8px; border-bottom:2px solid #444444">
		<div>
			<b><xsl:call-template name="format-date">
				<!--xsl:with-param name="date" select="substring-before(pubDate, ' +')"/-->
				<xsl:with-param name="date" select="pubDate"/>
			</xsl:call-template>:
			<xsl:value-of select="title" />
			</b>
		</div>
		<div>
			<!--xsl:value-of select="description" /-->
			<xsl:apply-templates select="description/*|description/text()" />
		</div>
	</div>
</xsl:template>

<!-- http://blog.beacontechnologies.com/xsl-for-formating-pubdate-from-rss-feed/ -->
<xsl:template name="format-date">
	<xsl:param name="date"/>
		<xsl:variable name="day" select="substring-before(substring-after($date, ' '), ' ')"/>
		<xsl:variable name="day2" select="concat(translate(substring($day,1,1), '0', ''), substring($day,2,1))"/>
		<xsl:variable name="monthName" select="substring-before(substring-after(substring-after($date, ' '), ' '), ' ')"/>
		<xsl:variable name="year" select="substring-before(substring-after(substring-after(substring-after($date, ' '), ' '), ' '), ' ')"/>
		<xsl:variable name="month">
		<xsl:choose>
			<xsl:when test="$monthName = 'Jan'">January</xsl:when>
			<xsl:when test="$monthName = 'Feb'">February</xsl:when>
			<xsl:when test="$monthName = 'Mar'">March</xsl:when>
			<xsl:when test="$monthName = 'Apr'">April</xsl:when>
			<xsl:when test="$monthName = 'May'">May</xsl:when>
			<xsl:when test="$monthName = 'Jun'">June</xsl:when>
			<xsl:when test="$monthName = 'Jul'">July</xsl:when>
			<xsl:when test="$monthName = 'Aug'">August</xsl:when>
			<xsl:when test="$monthName = 'Sep'">September</xsl:when>
			<xsl:when test="$monthName = 'Oct'">October</xsl:when>
			<xsl:when test="$monthName = 'Nov'">November</xsl:when>
			<xsl:when test="$monthName = 'Dec'">December</xsl:when>
			<xsl:otherwise/>
		</xsl:choose>
	</xsl:variable>
	<xsl:value-of select="concat($month, ' ', $day2, ', ', $year)"/>
</xsl:template>

</xsl:stylesheet>
