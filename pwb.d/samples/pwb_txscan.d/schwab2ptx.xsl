<?xml version="1.0" encoding="utf-8" ?>

<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    >
  <xsl:output
      method="text"
      version="1.0"
      indent="yes"
      encoding="utf-8"/>

  <xsl:variable name="delim" select="'&#09;'" />
  <xsl:variable name="nl" select="'&#10;'" />

  <!-- Converting to xs:dateTime, YYYY-MM-DDThh:mm:ss -->
  <xsl:template match="Date" mode="to_datetime">
    <xsl:value-of select="concat(substring(.,7,4),'-')" />
    <xsl:value-of select="concat(substring(.,1,2),'-')" />
    <xsl:value-of select="concat(substring(.,4,2),'T00:00:00')" />
  </xsl:template>

  <xsl:template match="BrokerageTransaction">
    <xsl:apply-templates select="Date" mode="to_datetime" />
    <xsl:value-of select="concat($delim, Amount)" />
    <xsl:value-of select="concat($delim, Description)" />
    <xsl:value-of select="concat($delim, Action)" />
    <xsl:value-of select="concat($delim, Symbol, $nl)" />
  </xsl:template>

  <xsl:template match="/">
    <xsl:value-of select="concat('[entries]', $nl)" />
    <xsl:apply-templates select="//BrokerageTransaction" />
  </xsl:template>

</xsl:stylesheet>
