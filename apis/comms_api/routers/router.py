from fastapi import APIRouter, Depends, status
from fastapi.responses import Response

from comms_api.authentication.authentication import JWTBearer
from comms_api.routers.authentication import authentication
from comms_api.routers.files import files

router = APIRouter(prefix='/api/v1')
router.add_api_route('/authentication', authentication, methods=['POST'])
router.add_api_route('/files', files, methods=['GET'], dependencies=Depends(JWTBearer))


@router.get('/')
async def home():
    return Response(status_code=status.HTTP_200_OK)
