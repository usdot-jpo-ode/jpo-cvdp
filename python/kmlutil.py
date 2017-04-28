from datetime import datetime
from lxml import etree
from pykml.factory import KML_ElementMaker as KML
from pykml.factory import GX_ElementMaker as GX

def _maketimespan(starttime, endtime):
    datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%SZ')
    return KML.TimeSpan(
        KML.begin(datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%SZ')),
        KML.end(datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%SZ'))
    )

class KMLTrip():
    def __init__(self, kmlcontainer, styletag=None, startime=None, endtime=None, stride=50):
        self.__kmlcontainer = kmlcontainer
        self.__styletag = styletag
        self.__startime = startime
        self.__endtime = endtime
        self.__stride = stride
        self.__currlat = None
        self.__currlng = None
        self.__prevplacemarklat = None
        self.__prevplacemarklng = None
        self.__coordstr = ''
        self.__isstride = False
        self.__pointcount = 0
        self.__totalpointcount = 0

    def makeplacemark(self):
        if self.__pointcount == 0:
            # We have no points since last placemark. Don't do anything.
            return

        if not self.__isstride:
            self.__coordstr += ' {},{},0 {},{},0'.format(
                self.__prevplacemarklng, 
                self.__prevplacemarklat, 
                self.__currlng, 
                self.__currlat
            )

        placemark = KML.Placemark()
        geometry = KML.MultiGeometry(
            KML.LineString(
                KML.tessellate('1'), 
                GX.altitudeMode('clampToGround'),
                KML.coordinates(
                    self.__coordstr
                )
            )
        )
        
        if self.__styletag is not None:
            placemark.append(KML.styleUrl('#' + str(self.__styletag)))

        placemark.append(geometry)

        if self.__startime is not None and self.__endtime is not None:
            placemark.append(_maketimespan(self.__startime, self.__endtime))

        self.__kmlcontainer.append(placemark)
        self.__coordstr = ''
        self.__pointcount = 0
        self.__prevplacemarklat = self.__currlat
        self.__prevplacemarklng = self.__currlng

    def setstyletag(self, styletag):
        self.__styletag = styletag

    def settimespan(self, starttime, endtime):
        self.__startime = starttime
        self.__endtime = endtime

    def addpoint(self, lat, lng):
        if self.__prevplacemarklat is None:
            self.__prevplacemarklat = lat
            self.__prevplacemarklng = lng

        if self.__currlat is not None and self.__pointcount % self.__stride == 0:
            self.__coordstr += ' {},{},0 {},{},0'.format(
                self.__prevplacemarklng, 
                self.__prevplacemarklat, 
                lng,
                lat
            )
            self.__prevplacemarklat = lat
            self.__prevplacemarklng = lng
            self.__isstride = True
        else:
            self.__isstride = False

        self.__pointcount += 1
        self.__totalpointcount += 1
        self.__currlat = lat
        self.__currlng = lng

def initkml(name):
    kmldoc = KML.Document(
        KML.name(name),
        KML.visibility('0'),
        KML.open('0')
    )

    return kmldoc

def initstyle(kmldoc, tag):
    style = KML.Style(id=str(tag))
    kmldoc.append(style)

    return style

def addlinestyle(kmlstyle, color, width):
    kmlstyle.append(
        KML.LineStyle(
            KML.color(color),
            KML.width(width)
        )     
    )

def addiconstyle(kmlstyle, iconref):
    kmlstyle.append(
        KML.IconStyle(
            KML.Icon(
                KML.href(iconref)
            )
        )
    )

def addpolystyle(kmlstyle, color, fill):
    kmlstyle.append(
        KML.PolyStyle(
            KML.color(color),
            KML.fill(fill)
        )
    )

def initfolder(kmldoc, foldername):
    folder = KML.Folder(
        KML.name(foldername),
        KML.visibility('0'),
        KML.open('0'),
    )
    
    kmldoc.append(folder)
    
    return folder

def addcircle(kmlcontainer, styletag, node, radius, name='circle', nsegments=50):
    if nsegments < 3:
        raise ValueError('Circle must be made up of 3 or more segments.')

    arclength = 360.0 / float(nsegments)
    currdegree = 0.0
    currnode = None
    coordstr = ''
    firstnode = None

    for i in range(nsegments):
        nextnode = node.destinationnode(
            currdegree,
            radius
        )
        
        if currnode is not None:
            coordstr += ' {},{},0 {},{},0'.format(
                currnode.lon, 
                currnode.lat, 
                nextnode.lon,
                nextnode.lat
            )
        else:
            firstnode = nextnode
        
        currnode = nextnode
        currdegree += arclength

    if firstnode is not None:
        coordstr += ' {},{},0 {},{},0'.format(
            currnode.lon, 
            currnode.lat, 
            firstnode.lon,
            firstnode.lat
        )
     
    placemark = KML.Placemark(
        KML.styleUrl('#' + str(styletag)),
        KML.name(name),
        KML.Polygon(
            KML.tessellate('1'), 
            GX.altitudeMode('clampToGround'),
            KML.outerBoundaryIs(
                KML.LinearRing(
                    KML.coordinates(coordstr)
                )
            )
        )
    )

    kmlcontainer.append(placemark)

def addsegment(kmlcontainer, styletag, lata, lnga, latb, lngb, name='segment'):
    coordstr = ' {},{},0 {},{},0'.format(lnga, lata, lngb, latb)

    placemark = KML.Placemark(
        KML.styleUrl('#' + str(styletag)),
        KML.name(name),
        KML.MultiGeometry(
            KML.LineString(
                KML.tessellate('1'), 
                GX.altitudeMode('clampToGround'),
                KML.coordinates(
                    coordstr
                )
            )
        )
    )

    kmlcontainer.append(placemark)

def addboundingbox(kmlcontainer, styletag, nelat, nelng, swlat, swlng):
    coordstr = ' {},{},0'.format(nelng, nelat)
    coordstr += ' {},{},0'.format(nelng, swlat)
    coordstr += ' {},{},0'.format(swlng, swlat)
    coordstr += ' {},{},0'.format(swlng, nelat)
    coordstr += ' {},{},0'.format(nelng, nelat)

    placemark = KML.Placemark(
        KML.styleUrl('#' + str(styletag)),
        KML.Polygon(
            KML.tessellate('1'), 
            GX.altitudeMode('clampToGround'),
            KML.outerBoundaryIs(
                KML.LinearRing(
                    KML.coordinates(coordstr)
                )
            )
        )
    )

    kmlcontainer.append(placemark)

def addpolygon(kmlcontainer, styletag, coordlist, name='polygon'):
    coordstr = ''
    first = None

    for coord in coordlist:
      if first is None:
        first = coord

      coordstr += ' {},{},0'.format(coord.lon, coord.lat)

    coordstr += ' {},{},0'.format(first.lon, first.lat)

    placemark = KML.Placemark(
        KML.name(name),
        KML.styleUrl('#' + str(styletag)),
        KML.Polygon(
            KML.tessellate('1'), 
            GX.altitudeMode('clampToGround'),
            KML.outerBoundaryIs(
                KML.LinearRing(
                    KML.coordinates(coordstr)
                )
            )
        )
    )

    kmlcontainer.append(placemark)

def addsegments(kmlcontainer, styletag, points, stride=50):
    pointiter = iter(points)
    
    try:
        initialpoint = next(pointiter)
    except StopIteration:
        raise ValueError('Number of points is 0')

    nsegmentsadded = 0
    currlat = initialpoint[0]
    currlng = initialpoint[1]
    coordstr = ''

    for i, point in enumerate(pointiter):
        if (i + 1) % stride != 0:
            continue
    
        nextlat = point[0]
        nextlng = point[1]

        coordstr += ' {},{},0 {},{},0'.format(
            currlng, 
            currlat, 
            nextlng,
            nextlat
        )

        currlat = nextlat
        currlng = nextlng

        nsegmentsadded += 1

    if nsegmentsadded == 0:
        coordstr += ' {},{},0 {},{},0'.format(
            initialpoint[1], 
            initialpoint[0], 
            currlng,
            currlat
        )

    placemark = KML.Placemark(
        KML.styleUrl('#' + str(styletag)),
        KML.MultiGeometry(
            KML.LineString(
                KML.tessellate('1'), 
                GX.altitudeMode('clampToGround'),
                KML.coordinates(
                    coordstr
                )
            )
        )
    )

    kmlcontainer.append(placemark)

def writekml(kmldoc, kmlpath):
    kmlio = etree.tostring(
        etree.ElementTree(KML.kml(kmldoc)), 
        pretty_print=True
    )

    with open(kmlpath, 'w') as kmlfile:
        kmlfile.write(str(kmlio))
