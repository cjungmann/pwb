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

  <!-- <xsl:variable name="delim" select="'&#127;'" /> -->
  <xsl:variable name="delim" select="'&#09;'" />
  <xsl:variable name="nl" select="'&#10;'" />

  <!-- Converting to xs:dateTime, YYYY-MM-DDThh:mm:ss -->
  <xsl:template match="dtposted" mode="to_datetime">
    <xsl:value-of select="concat(substring(.,1,4),'-')" />
    <xsl:value-of select="concat(substring(.,5,2),'-')" />
    <xsl:value-of select="concat(substring(.,7,2),'T')" />
    <xsl:value-of select="concat(substring(.,9,2),':')" />
    <xsl:value-of select="concat(substring(.,11,2),':')" />
    <xsl:value-of select="substring(.,13,2)" />
  </xsl:template>

  <xsl:template match="stmttrn" mode="tline">
    <xsl:apply-templates select="dtposted" mode="to_datetime" />
    <xsl:value-of select="concat($delim, trnamt)" />
    <xsl:value-of select="concat($delim, name)" />
    <xsl:value-of select="concat($delim, trntype)" />
    <xsl:value-of select="concat($delim, memo, $nl)" />
  </xsl:template>

  <xsl:template mode="meta" match="intu.userid">
    <xsl:value-of select="concat('user:', .)" />
  </xsl:template>

  <xsl:template mode="meta" match="bankacctfrom">
    <xsl:value-of select="concat(bankid, ' ', acctid, ' ', accttype)" />
  </xsl:template>


  <xsl:template match="/">
    <!-- <xsl:value-of select="concat('[meta]',$nl)" /> -->
    <!-- <xsl:apply-templates mode="meta" select="OFX" /> -->
    <xsl:value-of select="concat('[entries]',$nl)" />
    <xsl:apply-templates mode="tline" select="//stmttrn" />
  </xsl:template>

</xsl:stylesheet>
